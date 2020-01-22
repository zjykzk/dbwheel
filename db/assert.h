// Copyright (c) 2020
//

#ifndef DB_ASSERT_H
#define DB_ASSERT_H

#include <cassert>

#define ASSERTM(exp, msg) assert(((void) msg, exp))

#endif  // DB_ASSERT_H
