/**
 * @file binary.h
 * a class definition of linear::type::binary
 */

#ifndef LINEAR_TYPE_BINARY_H_
#define LINEAR_TYPE_BINARY_H_

#include <cstring>
#include <string>

#include "linear/msgpack_inc.h"

namespace linear {

namespace type {

/**
 * @class linear::type::binary binary.h "linear/binary.h"
 * represent binary object
 * @note
 * This class inherits std::string and almost same as std::string,
 * so you can use several methods for std::string.
 */
class binary : public std::string {
 public:
  binary() : std::string() {
  }
  binary(const char* ptr, size_t siz) : std::string(ptr, siz) {
  }
  template <class InputIterator>
  binary(InputIterator b, InputIterator e) : std::string(b, e) {
  }
  ~binary() {
  }

  /// @cond hidden
  template <typename Packer>
  void msgpack_pack(Packer& pk) const {
    pk.pack(msgpack::type::raw_ref(this->data(), this->size()));
  }
  void msgpack_unpack(msgpack::object o) {
    switch (o.type) {
    case msgpack::type::BIN:
        this->assign(o.via.bin.ptr, o.via.bin.size);
        break;
    case msgpack::type::STR:
        this->assign(o.via.str.ptr, o.via.str.size);
        break;
    case msgpack::type::NIL:
    case msgpack::type::BOOLEAN:
    case msgpack::type::POSITIVE_INTEGER:
    case msgpack::type::NEGATIVE_INTEGER:
    case msgpack::type::FLOAT:
    case msgpack::type::ARRAY:
    case msgpack::type::MAP:
    case msgpack::type::EXT:
    default:
        throw msgpack::type_error();
        break;
    }
  }
  template <typename MSGPACK_OBJECT>
  void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone& z) const {
    o->type = msgpack::type::BIN;
    char* ptr = static_cast<char*>(z.allocate_align(this->size()));
    o->via.bin.ptr = ptr;
    o->via.bin.size = static_cast<uint32_t>(this->size());
    memcpy(ptr, this->data(), this->size());
  }
  /// @endcond
};

/**
 * @fn binary(const char* ptr, size_type size)
 * constructor
 * @memberof linear::type::binary
 * @param ptr pointer to data
 * @param size size of data
 */

/**
 * @fn size_type size()
 * returns size of binary object
 * @memberof linear::type::binary
 * @return size of binary object
 */

/**
 * @fn const char* data()
 * returns pointer to first data
 * @memberof linear::type::binary
 * @return pointer to first data
 */

/**
 * @fn iterator begin()
 * returns iterator to first data
 * @memberof linear::type::binary
 * @return iterator to first data
 */

/**
 * @fn iterator end()
 * returns iterator to last data
 * @memberof linear::type::binary
 * @return iterator to last data
 */

} // namespace type

} // namespace linear

#endif // LINEAR_TYPE_BINARY_H_
