//
//  ex20_dbg.h
//
//valgrind --gen-suppressions=yes ./ex
//valgrind --suppressions=objc.supp ./ex

#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

/*if: lets you recompile your program so that all the debug log messages are removed
    
    if you compile w/ NDEBUG defined
        "no debug" messages will remain
    else
        alternative '#define debug' that translates any use of 'debug("format", arg1, arg2)' into an 'fprintf' call to 'stderr'
 */

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

//clean_errno: macro used to get a safe readable version of errno
#define clean_errno() (errno == 0? "None" : strerror(errno))

//The 'log_err', 'log_warn', and 'log_info': macros for logging messages meant for the end user. Works like 'debug' but can't be compiled out.

#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

//check: will make sure the condition @A is true, and if not logs the error @M (with variable arguments for 'log_err'), then jumps to the function's error: for cleanup.
#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno = 0; goto error; }

//sentinel: is placed in any part of a function that shouldn't run, and if it does prints an error message then jumps to the 'error:' label. You put this in 'if-statements' and 'switch-statements' to catch conditions that shouldn't happen, like the 'default:'
#define sentinel(M, ...) { log_err(M, ##__VA_ARGS__); errno = 0; goto error; }

//check_mem: makes sure a pointer is valid, and if it isn't reports it as an error with "Out of memory."
#define check_mem(A) check((A), "Out of memory.")

//check_debug: alternative that still checks/handles an error but doesn't report common errors. Uses 'debug'(not 'log_err') to report the message so the check still happens w/ 'NDEBUG' defined. Then the error jump goes off, but the message isn't printed.
#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno = 0; goto error; }

#endif








