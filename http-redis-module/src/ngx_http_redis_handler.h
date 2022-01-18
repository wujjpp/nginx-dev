
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */


#ifndef _NGX_HTTP_REDIS_HANDLER_H_INCLUDED_
#define _NGX_HTTP_REDIS_HANDLER_H_INCLUDED_

#include "ngx_http_redis_module.h"

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

ngx_int_t
ngx_http_redis_handler(ngx_http_request_t *r);

#endif