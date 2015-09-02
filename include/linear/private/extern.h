/**
 * @file extern.h
 * __declspec for WIN32 etc. 
 */

#ifndef LINEAR_PRIVATE_EXTERN_H_
#define LINEAR_PRIVATE_EXTERN_H_

// refer to
// <URL: http://msdn.microsoft.com/ja-jp/library/3y1sfaz2.aspx>
// <URL: http://zone.ni.com/reference/en-XX/help/370052H-01/tsref/infotopics/exporting_in_net/>

/* Windows - set up dll import/export decorators. */
#ifdef _WIN32

// refer to
// <URL: https://social.msdn.microsoft.com/Forums/vstudio/ja-JP/c20b6c84-0489-485f-a1bc-d7140ec6c683/warning-c4251?forum=vcgeneralja>
# pragma warning(disable: 4251)

# if defined(BUILDING_LINEAR_SHARED)
   /* Building shared library. */
#  define LINEAR_EXTERN __declspec(dllexport)
# elif defined(USING_LINEAR_SHARED)
   /* Using shared library. */
#  define LINEAR_EXTERN __declspec(dllimport)
# else
   /* Building static library. */
#  define LINEAR_EXTERN /* nothing */
# endif

/* xNix */
#elif __GNUC__ >= 4
# define LINEAR_EXTERN __attribute__((visibility("default")))
#else
# define LINEAR_EXTERN /* nothing */
#endif

#endif // LINEAR_PRIVATE_EXTERN_H_
