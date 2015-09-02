/**
 * @file group.h
 * Group class definition
 **/
#ifndef LINEAR_GROUP_H_
#define LINEAR_GROUP_H_

#include <vector>
#include <set>

#include "linear/socket.h"

/**
 * BROADCAST Group definition\n
 * not use this string for group name at application
 **/
#define LINEAR_BROADCAST_GROUP "$_broadcast_$"

namespace linear {

/**
 * @class Group group.h "linear/group.h"
 * A class that manages the group of Sockets for multicast or broadcast.
 * @see LINEAR_BROADCAST_GROUP
 */
class LINEAR_EXTERN Group {
 public:
  /**
   * get list of group names.
   * @return group name vector
   */
  static std::vector<std::string> Names();
  /**
   * get linear::Socket that belongs to the specific group.
   * @param name group name
   * @return socket set
   */
  static std::set<linear::Socket> Get(const std::string& name);
  /**
   * joins the specific linear::Socket to the specific group.
   * @param name group name
   * @param socket linear::Socket
   */
  static void Join(const std::string& name, const linear::Socket& socket);
  /**
   * leaves the specific linear::Socket from the specific group.
   * @param name group name
   * @param socket linear::Socket
   */
  static void Leave(const std::string& name, const linear::Socket& socket);
  /**
   * leaves the specific linear::Socket from all groups.
   * @param socket linear::Socket
   */
  static void LeaveAll(const linear::Socket& socket);
};

}  // namespace linear

#endif  // LINEAR_GROUP_H_
