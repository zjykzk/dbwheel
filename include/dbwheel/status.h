// Copyright (c) 2020
//
#ifndef DBWHEEL_INCLUDE_STATUS_H_
#define DBWHEEL_INCLUDE_STATUS_H_

#include <string>

namespace dbwheel {

class Status {
 public:
  static Status ioError(const std::string& msg) { return Status{kIOError, msg}; }
  static Status sysError(const std::string& msg) { return Status{kSysError, msg}; }
  static Status dataError(const std::string& msg) { return Status{kDataError, msg}; }
  static Status OK() { return Status{kOk, ""}; }

  bool ok() const { return code_ == kOk; }
  bool isIOError() const { return code_ == kIOError; }
  bool isSysError() const { return code_ == kSysError; }

  std::string toString() const;

 private:
  enum Code {
    kOk = 0,
    kIOError = 1,
    kSysError = 2,
    kDataError = 3,
  };

  Status(Code code, const std::string& msg): code_(code), msg_(msg) {}

  Code code_;
  std::string msg_;
};

}  // namespace dbwheel

#endif  // DBWHEEL_INCLUDE_STATUS_H
