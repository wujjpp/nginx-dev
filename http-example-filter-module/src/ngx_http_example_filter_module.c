/**
 * Created by Wu Jian Ping on - 2022/01/14.
 */

#include "ddebug.h"

#include "ngx_http_example_filter_module.h"

static ngx_int_t
ngx_http_example_header_filter(ngx_http_request_t *r);

static ngx_int_t
ngx_http_example_body_filter(ngx_http_request_t *r, ngx_chain_t *in);

static ngx_int_t
ngx_http_example_filter_init(ngx_conf_t *cf);

static void *
ngx_http_example_filter_create_loc_conf(ngx_conf_t *cf);

static char *
ngx_http_example_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_command_t ngx_http_example_filter_commands[] = {

    { ngx_string("example_filter"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_filter_loc_conf_t, enable),
      NULL },

    ngx_null_command
};


static ngx_http_module_t ngx_http_example_filter_ctx = {
    NULL,
    ngx_http_example_filter_init,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_example_filter_create_loc_conf,
    ngx_http_example_filter_merge_loc_conf,
};


ngx_module_t ngx_http_example_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_example_filter_ctx,     /* module context */
    ngx_http_example_filter_commands, /* module directives */
    NGX_HTTP_MODULE,                  /* module type */
    NULL,                             /* init master */
    NULL,                             /* init module */
    NULL,                             /* init process */
    NULL,                             /* init thread */
    NULL,                             /* exit thread */
    NULL,                             /* exit process */
    NULL,                             /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt ngx_http_next_body_filter;


static ngx_int_t
ngx_http_example_header_filter(ngx_http_request_t *r)
{
    debug_print_str("liftcycle: ngx_http_example_header_filter called");
    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_example_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    debug_print_str("liftcycle: ngx_http_example_body_filter called");
    return ngx_http_next_body_filter(r, in);
}

static ngx_int_t
ngx_http_example_filter_init(ngx_conf_t *cf)
{
    debug_print_str("liftcycle: ngx_http_example_filter_init called");
    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter  = ngx_http_example_body_filter;

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter  = ngx_http_example_header_filter;

    return NGX_OK;
}

static void *
ngx_http_example_filter_create_loc_conf(ngx_conf_t *cf)
{
    debug_print_str("liftcycle: ngx_http_example_filter_create_loc_conf called");
    ngx_http_example_filter_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_filter_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_example_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    debug_print_str("liftcycle: ngx_http_example_filter_merge_loc_conf called");

    ngx_http_example_filter_loc_conf_t *prev = parent;
    ngx_http_example_filter_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}