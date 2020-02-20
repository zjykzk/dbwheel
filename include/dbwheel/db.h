// Copyright (c) 2020
//
#ifndef DBWHEEL_INCLUDE_DB_H_
#define DBWHEEL_INCLUDE_DB_H_

#include "include/dbwheel/options.h"
#include "include/dbwheel/status.h"

namespace dbwheel {

class TX;

class DB {
 public:
  // Open the database with the specified 'name'
  // Stores a pointer to a heap-allocated database in *dbptr and returns
  // OK on success.
  //
  // Stores nullptr in *dbptr and returns a non-OK status on error.
  //
  // Caller should delete *dbptr when it is no longer needed.
  static Status open(const Options& options, const std::string& dbname, DB** dbptr);

  DB() = default;

  virtual ~DB();

  virtual Status close() = 0;
  virtual Status update(void (*f)(TX*)) = 0;
};

}  // namespace dbwheel

#endif  // DBWHEEL_INCLUDE_OPTIONS_H
