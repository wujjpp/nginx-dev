/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#include "ddebug.h"

#include "ngx_http_example_access_handler.h"

ngx_int_t
ngx_http_example_access_handler(ngx_http_request_t *r)
{
    ngx_http_example_access_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_example_access_module);
    if (conf->allow)
        return NGX_OK;

    return NGX_HTTP_FORBIDDEN;
}
