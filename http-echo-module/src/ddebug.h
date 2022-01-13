/**
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

#    define debug_print_ngx_str_t(str)               \
        {                                            \
            u_char buffer[1024];                     \
            ngx_memzero(buffer, sizeof(buffer));     \
            ngx_memcpy(buffer, str->data, str->len); \
            ngx_write_stdout((char *)buffer);        \
            ngx_write_stdout("\n");                  \
        }

#    define debug_print_str(str)    \
        {                           \
            ngx_write_stdout(str);  \
            ngx_write_stdout("\n"); \
        }

#else

#    define debug_print_ngx_str_t(str)
#    define debug_print_str(str)

#endif
#endif /* _DDEBUG_H__INCLUDED_ */
