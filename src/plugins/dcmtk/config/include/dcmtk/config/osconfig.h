#ifndef OSCONFIG_H
#define OSCONFIG_H

/*
** Define enclosures for include files with C linkage (mostly system headers)
*/
#ifdef __cplusplus
#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif


/*
** This head includes an OS/Compiler specific configuration header.
** Add entries for specific non-unix OS/Compiler environments.
** Under unix the default <cfunix.h> should be used.
**
*/

#if defined(_WIN32)
/*
** Visual C++ in a Windows 32 bit environment (Windows 9x/Me/NT/2000/XP)
*/
#include "dcmtk/config/cfwin32.h"

// mingw  overrides by dov
#define USE_STD_CXX_INCLUDES 1
#define HAVE_SSTREAM 1
#undef HAVE_IOS_NOCREATE
#define HAVE_DECLARATION_STD__IOS_BASE__OPENMODE 1
#define HAVE_EXPLICIT_TEMPLATE_SPECIALIZATION 1

using namespace std;
#else

/* By default assume Linux.
*/
#include "cfunix.h"
// mingw  overrides by dov
#define __ssize_t_defined
#define HAVE_SSTREAM 1
#undef HAVE_IOS_NOCREATE
#define HAVE_DECLARATION_STD__IOS_BASE__OPENMODE 1
#define HAVE_CLASS_TEMPLATE 1
#define HAVE_FUNCTION_TEMPLATE 1
#define HAVE_DIRENT_H 1
#define HAVE_PTHREAD_H 1
#define HAVE_FORK 1
#define HAVE_EXPLICIT_TEMPLATE_SPECIALIZATION 1

using namespace std;
#endif

#endif /* !OSCONFIG_H*/
