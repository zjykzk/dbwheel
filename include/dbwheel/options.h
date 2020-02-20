// Copyright (c) 2020
//
#ifndef DBWHEEL_INCLUDE_OPTIONS_H_
#define DBWHEEL_INCLUDE_OPTIONS_H_

namespace dbwheel {

struct Options {
  int initialMmapSize;
  int mmapFlags;
  bool readOnly;
};

}  // namespace dbwheel

#endif  // DBWHEEL_INCLUDE_OPTIONS_H
