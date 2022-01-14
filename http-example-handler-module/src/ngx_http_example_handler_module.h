/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#ifndef _NGX_HTTP_EXAMPLE_HANDLER_MODULE_H_INCLUDED_
#define _NGX_HTTP_EXAMPLE_HANDLER_MODULE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t message;
    ngx_flag_t counter;
} ngx_http_example_handler_loc_conf_t;

extern ngx_module_t ngx_http_example_handler_module;

#endif
