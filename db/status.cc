#include "include/dbwheel/status.h"

#include <sstream>

#include "db/assert.h"

namespace dbwheel {

std::string Status::toString() const {

  std::stringstream s;
  switch (code_) {
    case kOk:
      return "ok";
    case kIOError:
      s << "ioerror:";
      break;
    case kSysError:
      s << "sys error:";
      break;
    case kDataError:
      s << "data error:";
      break;
    default:
      s << "uknown code:" << code_;
      ASSERTM(false, s.str());
  }

  s << msg_;

  return s.str();
}

}  // namespace dbwheel
