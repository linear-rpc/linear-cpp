#include <sstream>

#include "linear/any.h"

namespace linear {

namespace type {

std::string any::stringify(int length, bool printable) const {
  std::ostringstream os;
  os << object_;
  if (printable) {
    std::string printable_string = os.str();
    if (length >= 0 && printable_string.size() > static_cast<size_t>(length)) {
      printable_string = printable_string.substr(0, length) + "...(truncated)\"";
    }
    for (std::string::iterator it = printable_string.begin(); it != printable_string.end(); it++) {
      if (!isprint(static_cast<unsigned char>(*it))) {
        *it = '?';
      }
    }
    return printable_string;
  } else {
    const std::string& raw_string = os.str();
    if (length >= 0 && raw_string.size() > static_cast<size_t>(length)) {
      return raw_string.substr(0, length) + "...(truncated)\"";
    } else {
      return raw_string;
    }
  }
}

}  // namespace type

}  // namespace linear
