
/*
 * Created by Wu Jian Ping on - 2022/01/18.
 */


#include "ngx_http_redis_handler.h"

ngx_int_t
ngx_http_redis_handler(ngx_http_request_t *r)
{
    ngx_http_redis_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_redis_module);

    if (conf == NULL)
        return NGX_HTTP_INTERNAL_SERVER_ERROR;

    return NGX_HTTP_BAD_REQUEST;
}
