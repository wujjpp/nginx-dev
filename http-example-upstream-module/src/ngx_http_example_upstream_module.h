
/*
 * Created by Wu Jian Ping on - 2022/01/17.
 */


#ifndef _NGX_HTTP_EXAMPLE_UPSTREAM_MODULE_H_INCLUDED_
#define _NGX_HTTP_EXAMPLE_UPSTREAM_MODULE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_http_upstream_conf_t upstream;
    ngx_int_t index;
    ngx_str_t password;
    ngx_uint_t db;
} ngx_http_example_upstream_loc_conf_t;

typedef struct {
    ngx_http_request_t *request;
} ngx_http_example_upstream_ctx_t;


extern ngx_module_t ngx_http_example_upstream_module;

#endif
