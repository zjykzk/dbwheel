// Copyright (c) 2020
//
#ifndef DB_OPTIONS_H_
#define DB_OPTIONS_H_

namespace dbwheel {

struct Options {
  int mmapFlags;
  bool readOnly;
};

}  // namespace dbwheel

#endif  // DB_OPTIONS_H

