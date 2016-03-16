#ifndef LINEAR_MSGPACK_INC_H_
#define LINEAR_MSGPACK_INC_H_

#ifdef _WIN32
// prevent to include winsock.h
# include <winsock2.h>

// surpress warnings related to msgpack
# pragma warning(push)
// http://msdn.microsoft.com/ja-jp/library/6kck0s93(v=vs.80).aspx
# pragma warning(disable: 4267)
// http://msdn.microsoft.com/ja-jp/library/sa28fef8%28VS.80%29.aspx
# pragma warning(disable: 4290)
// http://msdn.microsoft.com/ja-jp/library/sz5z1byt(v=vs.80).aspx
# pragma warning(disable: 4309)
#else
# pragma GCC system_header // TODO: for gcc (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3...
#endif

// use new version of msgpack
#define MSGPACK_DISABLE_LEGACY_NIL	(1)
#define MSGPACK_DISABLE_LEGACY_CONVERT	(1)

#include "msgpack.hpp"

#ifdef _WIN32
# pragma warning(pop)
#endif

#endif  // LINEAR_MSGPACK_INC_H_
