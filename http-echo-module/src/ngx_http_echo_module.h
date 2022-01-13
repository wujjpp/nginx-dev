/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#ifndef _NGX_HTTP_ECHO_MODULE_H_INCLUDED_
#define _NGX_HTTP_ECHO_MODULE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t message;
} ngx_http_echo_loc_conf_t;

extern ngx_module_t ngx_http_echo_module;

#endif
