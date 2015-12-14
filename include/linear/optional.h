/**
 * @file optional.h
 * a class definition of linear::type::optional
 */

#ifndef LINEAR_TYPE_OPTIONAL_H_
#define LINEAR_TYPE_OPTIONAL_H_

#include <cassert>
#include <cstring>
#include <functional>
#include <stdexcept>

#include "linear/nil.h"

#include "linear/private/extern.h"

namespace linear {

namespace type {

/**
 * @class optional optional.h "linear/optional.h"
 * this class can be stored nil or object.
 *
 * @warning this class can not be stored reference object.
 */
template <typename T>
class LINEAR_EXTERN optional {
 public:
  optional() : initialized_(false) {
  }
  optional(linear::type::nil) : initialized_(false) {
  }
  optional(const optional<T>& other) {
    initialized_ = other.initialized_;
    if (other.initialized_) {
      value_ = other.value_;
    }
  }
  optional(const T& value) : initialized_(true), value_(value) {
  }
  ~optional() {
  }

  optional& operator=(linear::type::nil) {
    initialized_ = false;
    return *this;
  }
  optional& operator=(const optional<T>& value) {
    if (!initialized_ && !value.initialized_) {
      return *this;
    }
    if (initialized_ && !value.initialized_) {
      initialized_ = false;
      return *this;
    }
    initialized_ = true;
    value_ = value.value_;
    return *this;
  }
  template <typename U>
  optional& operator=(const U& value) {
    initialized_ = true;
    value_ = value;
    return *this;
  }

  const T* operator->() const {
    assert(initialized_);
    return &value_;
  }
  T* operator->() {
    assert(initialized_);
    return &value_;
  }
  const T& operator*() const {
    assert(initialized_);
    return value_;
  }
  T& operator*() {
    assert(initialized_);
    return value_;
  }

  operator bool() const {
    return initialized_;
  }

  const T& value() const {
    if (!initialized_) {
      throw std::logic_error(std::string("*this is in disengaged state."));
    }
    return value_;
  }
  T& value() {
    if (!initialized_) {
      throw std::logic_error(std::string("*this is in disengaged state."));
    }
    return value_;
  }

  template <typename U>
  T value_or(const U& value) const {
    if (!initialized_) {
      return value;
    } else {
      return value_;
    }
  }

  /// @cond DEV
  template <typename Packer>
  void msgpack_pack(Packer& pk) const {
    if (initialized_) {
      pk.pack(value_);
    } else {
      pk.pack(linear::type::nil());
    }
  }
  void msgpack_unpack(msgpack::object o) {
    switch (o.type) {
    case msgpack::type::NIL:
      initialized_ = false;
      break;
    case msgpack::type::BOOLEAN:
    case msgpack::type::POSITIVE_INTEGER:
    case msgpack::type::NEGATIVE_INTEGER:
    case msgpack::type::FLOAT:
    case msgpack::type::BIN:
    case msgpack::type::STR:
    case msgpack::type::EXT:
    case msgpack::type::ARRAY:
    case msgpack::type::MAP:
      initialized_ = true;
      o.convert(&value_);
      break;
    }
  }
  template <typename MSGPACK_OBJECT>
  void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone& z) const {
    if (initialized_) {
      msgpack::object src = msgpack::object(value_, z);
      copy_msgpack_object(src, o, z);
    } else {
      o->type = msgpack::type::NIL;
    }
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
    case msgpack::type::EXT: {
      char* ptr = static_cast<char*>(z.allocate_align(src.via.str.size + 1));
      dst->via.str.ptr = ptr;
      dst->via.str.size = src.via.str.size;
#ifdef _WIN32
      memcpy_s(ptr, dst->via.str.size, src.via.str.ptr, src.via.str.size + 1);
#else
      memcpy(ptr, src.via.str.ptr, src.via.str.size + 1);
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
    }
  }

  bool initialized_;
  T value_;
};

}  // namespace type

}  // namespace linear

template <class T>
bool operator==(const linear::type::optional<T>& lhs, const linear::type::optional<T>& rhs) {
  if (bool(lhs) != bool(rhs)) {
    return false;
  } else if (!bool(lhs) && !bool(rhs)) {
    return true;
  } else {
    return (*lhs == *rhs);
  }
}
template <class T>
bool operator<(const linear::type::optional<T>& lhs, const linear::type::optional<T>& rhs) {
  if (bool(rhs) == false) {
    return false;
  } else if (bool(lhs) == false) {
    return true;
  } else {
    return std::less<T>()(*lhs, *rhs);
  }
}

template <class T>
bool operator==(const linear::type::optional<T>& opt, linear::type::nil) {
  return !bool(opt);
}
template <class T>
bool operator==(linear::type::nil, const linear::type::optional<T>& opt) {
  return false;
}
template <class T>
bool operator<(const linear::type::optional<T>& opt, linear::type::nil) {
  return !bool(opt);
}
template <class T>
bool operator<(linear::type::nil, const linear::type::optional<T>& opt) {
  return bool(opt);
}

template <class T>
bool operator==(const linear::type::optional<T>& opt, const T& value) {
  return (bool(opt) ? (*opt == value) : false);
}
template <class T>
bool operator==(const T& value, const linear::type::optional<T>& opt) {
  return (bool(opt) ? (*opt == value) : false);
}
template <class T>
bool operator<(const linear::type::optional<T>& opt, const T& value) {
  return (bool(opt) ? std::less<T>()(*opt, value) : true);
}

#endif  // LINEAR_TYPE_OPTIONAL_H_
