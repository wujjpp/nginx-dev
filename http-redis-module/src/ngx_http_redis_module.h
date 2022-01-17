
/*
 * Created by Wu Jian Ping on - 2022/01/17.
 */


#ifndef _NGX_HTTP_REDIS_H_INCLUDED_
#define _NGX_HTTP_REDIS_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t redis_addr;
} ngx_http_redis_loc_conf_t;

#endif
