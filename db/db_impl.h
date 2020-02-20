// Copyright (c) 2020
//
#ifndef DB_DB_IMPL_H_
#define DB_DB_IMPL_H_

#include <cstdint>
#include <string>
#include <utility>

#include "include/dbwheel/db.h"

namespace dbwheel {

class Meta;
class Page;

class DBImpl : public DB {
 public:

  DBImpl(const Options& options, const std::string& dbname):
    name_(dbname),
    options_(options),
    open_(true) {}
  ~DBImpl() override;
  Status update(void (*f)(TX*)) override;

  Status open();
  Status close() override;
  Page* page(uint64_t pageID);

 private:
  
  Status openFile();
  Status init();
  Status mmapFile();
  std::pair<uint64_t, Status> mmapSize(uint64_t size);
  Status readMeta();

  std::string name_;
  Options options_;
  bool open_;
  int fd_;
  int pageSize_;
  char* data_;
  uint64_t dataSize_;
  Meta* meta0_;
  Meta* meta1_;
};

}  // namespace dbwheel

#endif  // DB_DB_IMPL_H
