#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([linear], [3.0.6], [https://github.com/linear-rpc/linear-cpp])
AC_CONFIG_AUX_DIR(m4)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_SRCDIR([include/linear/handler.h])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h])

# linear-version-id
LINEAR_VERSION_ID="`echo $PACKAGE_NAME-$PACKAGE_VERSION`"
# AC_DEFINE_UNQUOTED([LINEAR_VERSION_ID], ["$PACKAGE_NAME-$PACKAGE_VERSION"], [linear-version-id])
AC_SUBST(LINEAR_VERSION_ID)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_CHECK_PROGS([DOXYGEN], [doxygen])
AC_SUBST(DOXYGEN)
AC_CHECK_PROGS([PERL], [perl])
if test -z "${PERL}"; then
   AC_MSG_ERROR([needs perl])
fi
AC_SUBST(PERL)
AC_CHECK_PROGS([AWK], [awk])
if test -z "${AWK}"; then
   AC_MSG_ERROR([needs awk])
fi
AC_SUBST(AWK)

AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([rt], [timer_create])

# TODO: add more warning options.
CFLAGS="$CFLAGS -Wall -Wno-unused-function -Werror"
CXXFLAGS="$CXXFLAGS -Wall -Wno-unused-function -Werror"

# for MacPorts
AC_CANONICAL_HOST
case $host_os in
    darwin*)
	is_darwin="yes"
        ;;
    *)
        ;;
esac
if test "x$is_darwin" = "xyes" -a -d /opt/local/include; then
   CFLAGS="$CFLAGS -I/opt/local/include"
   CXXFLAGS="$CXXFLAGS -I/opt/local/include"
   LDFLAGS="$LDFLAGS -L/opt/local/lib"
fi

AC_MSG_CHECKING(whether --enable-uvheader option specified)
AC_ARG_ENABLE([uvheader],
              AC_HELP_STRING([--enable-uvheader], [install uv header@<:@default=no@:>@]),
              [uvheader=$enableval], [uvheader=no])
AC_MSG_RESULT(${uvheader})
if test "x${uvheader}" != "xno"; then
   AC_SUBST(ENABLE_UVHEADER, 1)
else
   AC_SUBST(ENABLE_UVHEADER, 0)
fi

# Checks for --enable-debug
# Appear LINEAR_DEBUG macro only when --enable-debug=dev
AC_MSG_CHECKING(whether --enable-debug option specified)
AC_ARG_ENABLE([debug],
              AC_HELP_STRING([--enable-debug], [enable debug@<:@default=yes@:>@]),
              [debug=$enableval], [debug=yes])
AC_MSG_RESULT(${debug})
if test "x${debug}" != "xno"; then
   CXXFLAGS="$CXXFLAGS -g"
else
   CXXFLAGS="$CXXFLAGS -O3"
fi
if test "x${debug}" = "xdev"; then
   CXXFLAGS="$CXXFLAGS -D_LINEAR_LOG_DEBUG_"
fi
AM_CONDITIONAL(LINEAR_LOG_DEBUG, test "x${debug}" = "xdev")

# Check for shared_ptr
AC_LANG([C++])
AC_MSG_CHECKING([for std::shared_ptr])
# not use boost now
AC_SUBST(HAVE_BOOST_SHARED_PTR, 0)
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <memory>]],
                     [[std::shared_ptr<int> test_shared_ptr;]])],
    [AC_MSG_RESULT([yes])
     # AC_DEFINE_UNQUOTED([HAVE_STD_SHARED_PTR], 1, [Define to 1 if you have std::shared_ptr.])
     AC_SUBST(HAVE_STD_SHARED_PTR, 1)],
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <tr1/memory>]],
                                        [[std::tr1::shared_ptr<int> test_shared_ptr;]])],
         [AC_MSG_RESULT([yes])
          # AC_DEFINE_UNQUOTED([HAVE_TR1_SHARED_PTR], 1, [Define to 1 if you have std::tr1::shared_ptr.])
          AC_SUBST(HAVE_TR1_SHARED_PTR, 1)],
         [AC_MSG_RESULT([no])
          AC_MSG_ERROR([configure cannot find shared_ptr.])])])
#         [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <boost/shared_ptr.hpp>]],
#                                             [[boost::shared_ptr<int> test_shared_ptr;]])],
#            [AC_MSG_RESULT([yes])
#             AC_DEFINE_UNQUOTED([HAVE_BOOST_SHARED_PTR], 1, [Define to 1 if you have boost::shared_ptr.])
#             AC_SUBST(HAVE_BOOST_SHARED_PTR, 1)],
#            [AC_MSG_RESULT([no])
#             AC_MSG_ERROR([configure cannot find shared_ptr.])])])])

# Checks for --with-ssl
AC_ARG_WITH([ssl],
            [AC_HELP_STRING([--with-ssl], [supports SSL transport by using OpenSSL@<:@default=no@:>@])],
            [with_ssl=$withval], [with_ssl=no])
if test "x$with_ssl" = "xyes"; then
   CXXFLAGS="$CXXFLAGS -DWITH_SSL"
   LIBS="-lcrypto -lssl $LIBS"
elif test "x${with_ssl}" != "xno"; then
   CXXFLAGS="$CXXFLAGS -DWITH_SSL -I$with_ssl/include"
   LDFLAGS="-L$with_ssl/lib $LDFLAGS"
   LIBS="-lcrypto -lssl $LIBS"
fi
AM_CONDITIONAL([WITH_SSL], [test "x${with_ssl}" != "xno"])

# Checks for --with-test
AC_ARG_WITH([test],
            AC_HELP_STRING([--with-test], [make tests@<:@default=yes@:>@]),
            [with_test=$withval], [with_test=no])
AM_CONDITIONAL([WITH_TEST], [test "x${with_test}" != "xno"])
if test "x${with_test}" != "xno"; then
   AC_SUBST(WITH_TEST, 1)
fi

# Checks for --with-sample
AC_ARG_WITH([sample],
            AC_HELP_STRING([--with-sample], [make samples@<:@default=no@:>@]),
            [with_sample=$withval], [with_sample=no])
AM_CONDITIONAL([WITH_SAMPLE], [test "x${with_sample}" != "xno"])
if test "x${with_sample}" != "xno"; then
   AC_SUBST(WITH_SAMPLE, 1)
fi

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/linear/Makefile
                 include/linear/private/Makefile
                 src/Makefile
                 test/Makefile
                 sample/Makefile
                 doc/Makefile])

AC_CONFIG_SUBDIRS([deps/msgpack deps/libtv])

if test "x${with_test}" != "xno"; then
   AC_CONFIG_SUBDIRS(deps/gmock-1.7.0)
fi

AC_CHECK_PROG(PKG_CONFIG, pkg-config, yes)
AM_CONDITIONAL([HAVE_PKG_CONFIG], [test "x$PKG_CONFIG" != "x"])
AS_IF([test "x$PKG_CONFIG" != "x"], [
    AC_CONFIG_FILES([liblinear.pc])
])

AC_OUTPUT
