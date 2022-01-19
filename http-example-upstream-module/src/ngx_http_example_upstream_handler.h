
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */


#ifndef _NGX_HTTP_EXAMPLE_UPSTREAM_HANDLER_H_INCLUDED_
#define _NGX_HTTP_EXAMPLE_UPSTREAM_HANDLER_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_http_example_upstream_module.h"

ngx_int_t ngx_http_example_upstream_handler(ngx_http_request_t *r);

#endif