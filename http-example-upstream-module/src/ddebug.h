
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#ifndef _DDEBUG_H__INCLUDED_
#define _DDEBUG_H__INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <time.h>

#ifndef DDEBUG
#    define DDEBUG 1
#endif

#if defined(DDEBUG) && (DDEBUG)

#    define debug_print_ngx_str_t(fmt, str)           \
        {                                             \
            u_char buffer[NGX_MAX_ERROR_STR] = { 0 }; \
            ngx_sprintf(buffer, fmt, (str)->data);    \
            ngx_write_stdout((char *)buffer);         \
        }

#    define debug_print(fmt, ...)                     \
        {                                             \
            u_char buffer[NGX_MAX_ERROR_STR] = { 0 }; \
            ngx_sprintf(buffer, fmt, __VA_ARGS__);    \
            ngx_write_stdout((char *)buffer);         \
        }

#    define debug_print_str(str)    \
        {                           \
            ngx_write_stdout(str);  \
            ngx_write_stdout("\n"); \
        }

#else

#    define debug_print_ngx_str_t(fmt, str)
#    define debug_print(fmt, ...)
#    define debug_print_str(str)

#endif
#endif /* _DDEBUG_H__INCLUDED_ */
