/**
 * @file any.h
 * a class definition of linear::type::any
 */

#ifndef LINEAR_TYPE_ANY_H_
#define LINEAR_TYPE_ANY_H_

#include "linear/binary.h"
#include "linear/optional.h"

#include "linear/private/extern.h"

namespace linear {

namespace type {

/**
 * @class any any.h "linear/any.h"
 * represent any type object
 */
class LINEAR_EXTERN any {
 public:
  //! any type indicator
  enum Type {
    NIL              = msgpack::type::NIL,              //!< NIL
    BOOLEAN          = msgpack::type::BOOLEAN,          //!< BOOLEAN
    POSITIVE_INTEGER = msgpack::type::POSITIVE_INTEGER, //!< POSITIVE_INTEGER
    NEGATIVE_INTEGER = msgpack::type::NEGATIVE_INTEGER, //!< NEGATIVE_INTEGER
    FLOAT            = msgpack::type::FLOAT,            //!< FLOAT
    STR              = msgpack::type::STR,              //!< STRING
    BIN              = msgpack::type::BIN,              //!< BINARY
    EXT              = msgpack::type::EXT,              //!< EXT
    ARRAY            = msgpack::type::ARRAY,            //!< ARRAY
    MAP              = msgpack::type::MAP               //!< MAP
  };

  /// @cond hidden
  any() : zone_(), object_(), type(NIL) {
  }
  any(const any& a) : zone_() {
    copy_msgpack_object(a.object_, &object_, zone_);
    type = static_cast<linear::type::any::Type>(object_.type);
  }
  any(const linear::type::nil&) : zone_(), object_(), type(NIL) {
  }
  any(const msgpack::object& o) : zone_() {
    copy_msgpack_object(o, &object_, zone_);
    type = static_cast<linear::type::any::Type>(object_.type);
  }
  template <typename Value>
  any(const Value& value) : zone_(), object_(value, &zone_), type(static_cast<linear::type::any::Type>(object_.type)) {
  }
  ~any() {
  }
  template <typename Value>
  any& operator=(const Value& value) {
    zone_.clear();
    object_ = msgpack::object(value, &zone_);
    type = static_cast<linear::type::any::Type>(object_.type);
    return *this;
  }
  any& operator=(const any& a) {
    zone_.clear();
    copy_msgpack_object(a.object_, &object_, zone_);
    type = static_cast<linear::type::any::Type>(object_.type);
    return *this;
  }
  any& operator=(const msgpack::object& o) {
    zone_.clear();
    copy_msgpack_object(o, &object_, zone_);
    type = static_cast<linear::type::any::Type>(object_.type);
    return *this;
  }
  bool operator<(const any& a) const {
    return (stringify() < a.stringify());
  }
  bool operator<=(const any& a) const {
    return (stringify() <= a.stringify());
  }
  bool operator>(const any& a) const {
    return (stringify() > a.stringify());
  }
  bool operator>=(const any& a) const {
    return (stringify() >= a.stringify());
  }
  bool operator==(const any& a) const {
    return (stringify() == a.stringify());
  }
  bool operator!=(const any& a) const {
    return (stringify() != a.stringify());
  }
  /**
   * get internal msgpack::object reference.
   * @warning return value is for referencing only.
   * @return msgpack::object
   */
  const msgpack::object& object() const {
    return object_;
  }
  /**
   * get internal msgpack::zone reference.
   * @warning return value is for referencing only.
   * @return msgpack::zone
   */
  const msgpack::zone& zone() const {
    return zone_;
  }
  /// @endcond

  /**
   * check nil or not
   * @return true if object is a nil object
   */
  bool is_nil() const {
    return object_.is_nil();
  }
  /**
   * downcast method to get concrete values
     @code
     std::map<std::string, linear::type::any> any_map;

     std::string key("integer");
     any_map.insert(std::make_pair<std::string, linear::type::any>(key, 1));

     key = "string";
     any_map.insert(std::make_pair<std::string, linear::type::any>(key, std::string("foo")));

     for (std::map<std::string, linear::type::any>::iterator it = any_map.begin();
          it != any_map.end(); it++) {
       if (it->first == "integer") {
         std::cerr << (it->second).as<int>() << std::endl;
       } else if (it->first == "string") {
         std::cerr << (it->second).as<std::string>() << std::endl;
       }
     }
     @endcode
   */
  template <typename Value>
  Value as() const {
    return object_.as<Value>();
  }
  /**
   * stringify any objects
   * @param length truncate the number of characters except when -1 is specified
   * @param printable replace !is_print char into '?'
   * @return stringified object
   */
  std::string stringify(int length = -1, bool printable = false) const;

  /// @cond hidden
  template <typename Packer>
  void msgpack_pack(Packer& pk) const {
    pk.pack(object_);
  }
  void msgpack_unpack(msgpack::object o) {
    zone_.clear();
    copy_msgpack_object(o, &object_, zone_);
  }
  template <typename MSGPACK_OBJECT>
  void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone& z) const {
    copy_msgpack_object(object_, o, z);
  }
  /// @endcond

 private:
  void copy_msgpack_object(const msgpack::object& src, msgpack::object* dst, msgpack::zone& z) const {
    dst->type = src.type;
    switch (src.type) {
    case msgpack::type::NIL:
      break;
    case msgpack::type::BOOLEAN:
      dst->via.boolean = src.via.boolean;
      break;
    case msgpack::type::POSITIVE_INTEGER:
      dst->via.u64 = src.via.u64;
      break;
    case msgpack::type::NEGATIVE_INTEGER:
      dst->via.i64 = src.via.i64;
      break;
    case msgpack::type::FLOAT:
      dst->via.f64 = src.via.f64;
      break;
    case msgpack::type::STR: {
      char* ptr = static_cast<char*>(z.allocate_align(src.via.str.size));
      dst->via.str.ptr = ptr;
      dst->via.str.size = src.via.str.size;
#ifdef _WIN32
      memcpy_s(ptr, dst->via.str.size, src.via.str.ptr, src.via.str.size);
#else
      memcpy(ptr, src.via.str.ptr, src.via.str.size);
#endif
      break;
    }
    case msgpack::type::BIN: {
      char* ptr = static_cast<char*>(z.allocate_align(src.via.bin.size));
      dst->via.bin.ptr = ptr;
      dst->via.bin.size = src.via.bin.size;
#ifdef _WIN32
      memcpy_s(ptr, dst->via.bin.size, src.via.bin.ptr, src.via.bin.size);
#else
      memcpy(ptr, src.via.bin.ptr, src.via.bin.size);
#endif
      break;
    }
    case msgpack::type::EXT: {
      char* ptr = static_cast<char*>(z.allocate_align(src.via.ext.size + 1));
      dst->via.ext.ptr = ptr;
      dst->via.ext.size = src.via.ext.size;
#ifdef _WIN32
      memcpy_s(ptr, dst->via.ext.size, src.via.ext.ptr, src.via.ext.size + 1);
#else
      memcpy(ptr, src.via.ext.ptr, src.via.ext.size + 1);
#endif
      break;
    }
    case msgpack::type::ARRAY:
      if (src.via.array.size == 0) {
        dst->via.array.ptr  = NULL;
        dst->via.array.size = 0;
      } else {
        msgpack::object* src_ptr = src.via.array.ptr;
        msgpack::object* dst_ptr = static_cast<msgpack::object*>(z.allocate_align(sizeof(msgpack::object) * src.via.array.size));
        dst->via.array.ptr = dst_ptr;
        dst->via.array.size = src.via.array.size;
        for (uint32_t i = 0; i < src.via.array.size; ++i) {
          copy_msgpack_object(*src_ptr, dst_ptr, z);
          ++src_ptr;
          ++dst_ptr;
        }
      }
      break;
    case msgpack::type::MAP:
      if (src.via.map.size == 0) {
        dst->via.map.ptr  = NULL;
        dst->via.map.size = 0;
      } else {
        msgpack::object_kv* src_ptr = src.via.map.ptr;
        msgpack::object_kv* dst_ptr = static_cast<msgpack::object_kv*>(z.allocate_align(sizeof(msgpack::object_kv) * src.via.map.size));
        dst->via.map.ptr = dst_ptr;
        dst->via.map.size = src.via.map.size;
        for (uint32_t i = 0; i < src.via.map.size; ++i) {
          copy_msgpack_object(src_ptr->key, &dst_ptr->key, z);
          copy_msgpack_object(src_ptr->val, &dst_ptr->val, z);
          ++src_ptr;
          ++dst_ptr;
        }
      }
      break;
    default:
      break;
    }
  }

  msgpack::zone   zone_;
  msgpack::object object_;

public:
  /**
   * any type indicator
   * @see linear::type::any::Type
   */
  Type type;
};

}  // namespace type

}  // namespace linear

#endif  // LINEAR_TYPE_ANY_H_
