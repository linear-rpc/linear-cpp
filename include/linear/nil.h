/**
 * @file nil.h
 * a class definition of linear::type::nil
 */

#ifndef LINEAR_TYPE_NIL_H_
#define LINEAR_TYPE_NIL_H_

#include "linear/msgpack_inc.h"

namespace linear {

/**
 * @namespace linear::type
 * namespace for type definitions
 **/
namespace type {

/**
 * @class nil nil.h "linear/nil.h"
 * represent nil object
 * @warning on iOS environment, you need to use nil_() instead of nil().
 */
class nil {
 public:
  /// @cond hidden
  nil() {
  }
  ~nil() {
  }
  template <typename Packer>
  void msgpack_pack(Packer& pk) const {
    pk.pack(msgpack::type::nil());
  }
  void msgpack_unpack(msgpack::object o) {
    if(o.type != msgpack::type::NIL) {
      throw msgpack::type_error();
    }
  }
  template <typename MSGPACK_OBJECT>
  void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone&) const {
    o->type = msgpack::type::NIL;
  }
  /// @endcond
};

} // namespace type

} // namespace linear

#endif // LINEAR_TYPE_NIL_H_
