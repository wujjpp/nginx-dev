
/*
 * Created by Wu Jian Ping on - 2022/01/14.
 */

#ifndef _NGX_HTTP_EXAMPLE_FILTER_MODULE_H_INCLUDED_
#define _NGX_HTTP_EXAMPLE_FILTER_MODULE_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_flag_t enable_header_filter;
    ngx_str_t prefix_message;
    ngx_str_t suffix_message;
} ngx_http_example_filter_loc_conf_t;

#endif
