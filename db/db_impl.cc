// Copyright (c) 2020
//

#include "db/db_impl.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "db/crc32c.h"
#include "db/bucket_impl.h"
#include "db/page.h"

namespace dbwheel {

int (*openFileFunc)(const char*, int, ...) = open;
int (*closeFileFunc)(int) = close;

// The data file format version.
static const uint32_t kVersion = 2;

// Represents a marker value to indicate that a file is a DB.
static const uint32_t kMagic = 0xED0CDAED;

// Represents max mmap size.
static const uint64_t kMaxMapSize = 0xFFFFFFFFFFFF;

// The size creating array pointer.
static const uint64_t kMaxAllocSize = 0x7FFFFFFF;

// The largest step that can be token when remapping the mmap.
static const uint32_t kMaxMapStep = 1 << 30;

inline static Status ioError() {
    return Status::ioError(strerror(errno));
}

struct Meta {
  uint32_t magic;
  uint32_t version;
  uint32_t pageSize;
  uint32_t flags;
  bucket root;
  uint64_t freelistPageID;
  uint64_t pageID;
  uint64_t txID;
  uint64_t checksum;

  void calcChecksum();
  bool validate();
};

void Meta::calcChecksum() {

    checksum = crc32c::Value(
        reinterpret_cast<char*>(this),
        reinterpret_cast<char*>(&checksum) - reinterpret_cast<char*>(this));
}

bool Meta::validate() {

    auto actual = crc32c::Value(
        reinterpret_cast<char*>(this),
        reinterpret_cast<char*>(&checksum) - reinterpret_cast<char*>(this));

    return actual == checksum;
}

Status DB::open(const Options& options, const std::string& dbname, DB** dbptr) {

  DBImpl* db = new DBImpl{options, dbname};

  Status s = db->open();
  if (!s.ok()) {
    delete db;
    db = nullptr;
  }

  *dbptr = db;

  return s;
}

Status DBImpl::open() {

  Status status = openFile();
  if (!status.ok()) {
    return status;
  }

  status = mmapFile();
  if (!status.ok()) {
    return status;
  }

  status = readMeta();
  if (!status.ok()) {
    return status;
  }

  return Status::OK();
}

Status DBImpl::openFile() {

  int flags = O_RDWR;
  if (options_.readOnly) {
    flags = O_RDONLY;
  }

  fd_ = openFileFunc(name_.data(), O_CREAT|flags, S_IROTH|S_IRGRP|S_IRUSR|S_IWUSR);

  if (fd_ == -1) {
    return ioError();
  }

  flags = LOCK_EX;
  if (options_.readOnly) {
    flags = LOCK_SH;
  }

  if (flock(fd_, flags | LOCK_NB) == -1) {
    return ioError();
  }

  struct stat sb;
  if (fstat(fd_, &sb) == -1) {
    return ioError();
  }

  if (sb.st_size == 0) {
    Status status = init();
    if (!status.ok()) {
      return status;
    }
    return status;
  }

  char buf[0x1000];
  if (read(fd_, buf, 0x1000) == -1) {
    return ioError();
  }
  auto m = (new (buf) Page{0, (uint16_t)0})->meta();
  pageSize_ = m->validate() ? m->pageSize : sysconf(_SC_PAGESIZE);

  return Status::OK();
}

Status DBImpl::init() {

  pageSize_ = sysconf(_SC_PAGESIZE);
  if (pageSize_ == -1) {
    return Status::sysError(strerror(errno)); 
  }

#define pageOf(buf, id, flags) new (buf) Page{id, static_cast<uint16_t>(flags)}

  size_t sz = pageSize_ * 4;
  char* buf = new char[sz];
  for (uint64_t i = 0; i < 2; i++) {
    Page* p = pageOf(buf + i * pageSize_, i, Page::kMetaPageFlag);
    Meta* m = p->meta();

    m->magic = kMagic;
    m->version = kVersion;
    m->pageSize = pageSize_;
    m->freelistPageID = 2;
    m->root.rootPageID = 3;
    m->pageID = 4;
    m->txID = i;
    m->checksum = crc32c::Value(
        reinterpret_cast<char*>(m),
        reinterpret_cast<char*>(&m->checksum) - reinterpret_cast<char*>(m));
  }

  // freelist page
  pageOf(buf, 2, Page::kFreeListPageFlag);
  // empty leaf page
  pageOf(buf, 3, Page::kLeafPageFlag);

  if (write(fd_, buf, sz) == -1) {
    delete [] buf;
    return ioError();
  }

  if (fsync(fd_) == -1) {
    delete [] buf;
    return ioError();
  }

  delete [] buf;
  return Status::OK();
}

Status DBImpl::mmapFile() {

  struct stat sb;
  if (fstat(fd_, &sb) == -1) {
    return ioError();
  }

  uint64_t size = sb.st_size;
  if (size < (uint64_t) options_.initialMmapSize) {
    size = options_.initialMmapSize;
  }

  auto ret = mmapSize(size);
  Status s = std::get<1>(ret);
  if (!s.ok()) {
    return s;
  }

  size = std::get<0>(ret);

  // TODO(zenk):
  // 1. dereference
  // 2. munmap

  data_ = static_cast<char*>(mmap(nullptr, size, PROT_READ, options_.mmapFlags|MAP_SHARED, fd_, 0));
  if (data_ == MAP_FAILED) {
    return Status::sysError(strerror(errno));
  }

  if (madvise(data_, size, MADV_RANDOM) < 0) {
    return Status::sysError(strerror(errno));
  }

  dataSize_ = size;

  return Status::OK();
}

std::pair<uint64_t, Status> DBImpl::mmapSize(uint64_t size) {

  for (uint64_t i = 15; i <= 30; i++) {
    if (size < (uint64_t)1 << i) {
      return std::make_pair(1<<i, Status::OK());
    }
  }

  if (size > kMaxMapSize) {
    return std::make_pair(0, Status::sysError("mmap size is over limit"));
  }

  auto r = size % kMaxMapStep;
  if (r > 0) {
    size += kMaxMapStep - r;
  }

  if (size % pageSize_ != 0) {
    size = (size / pageSize_ + 1) * pageSize_;
  }

  if (size > kMaxMapSize) {
    size = kMaxMapSize;
  }

  return std::make_pair(size, Status::OK());
}

Status DBImpl::readMeta() {

  meta0_ = page(0)->meta();
  meta1_ = page(1)->meta();

  if (!meta0_->validate() && !meta1_->validate()) {
    return Status::dataError("invalid meta data");
  }

  return Status::OK();
}

Status DBImpl::close() {

  // unlock
  if (!options_.readOnly && flock(fd_, LOCK_UN) == -1) {
    return ioError();
  }

  if (closeFileFunc(fd_) == -1) {
    return ioError();
  }

  return Status::OK();
}

Status DBImpl::update(void (*f)(TX* tx)) {

  return Status::OK();
}

inline Page* DBImpl::page(uint64_t pageID) {
  return reinterpret_cast<Page*>(data_ + pageID * pageSize_);
}

DB::~DB() = default;

DBImpl::~DBImpl() {

}

}  // namespace dbwheel

