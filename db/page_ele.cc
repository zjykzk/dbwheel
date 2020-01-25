#include "db/page_ele.h"

namespace dbwheel {

std::string branchPageElement::key() {
  return std::string((reinterpret_cast<char*>(this) + this->pos), this->ksize);
}

std::string leafPageElement::key() {
  return std::string((reinterpret_cast<char*>(this) + this->pos), this->ksize);
}

std::string leafPageElement::value() {
  return std::string((reinterpret_cast<char*>(this) + this->pos + this->ksize), this->vsize);
}

}  // namespace dbwheel
