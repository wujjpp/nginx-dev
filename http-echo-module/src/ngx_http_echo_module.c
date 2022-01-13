/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#include "ddebug.h"

#include "ngx_http_echo_handler.h"
#include "ngx_http_echo_module.h"

static void *
ngx_http_echo_create_loc_conf(ngx_conf_t *cf);

static char *
ngx_http_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static char *
ngx_http_proxy_hello(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_echo_commands[] = {

    { ngx_string("proxy_hello"),
      NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
      ngx_http_proxy_hello,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("set_proxy_hello_message"),
      NGX_HTTP_LOC_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_echo_loc_conf_t, message),
      NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_echo_ctx = {
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_echo_create_loc_conf,
    ngx_http_echo_merge_loc_conf
};

ngx_module_t ngx_http_echo_module = {
    NGX_MODULE_V1,
    &ngx_http_echo_ctx,     /* module context */
    ngx_http_echo_commands, /* module directives */
    NGX_HTTP_MODULE,        /* module type */
    NULL,                   /* init master */
    NULL,                   /* init module */
    NULL,                   /* init process */
    NULL,                   /* init thread */
    NULL,                   /* exit thread */
    NULL,                   /* exit process */
    NULL,                   /* exit master */
    NGX_MODULE_V1_PADDING
};

static char *
ngx_http_proxy_hello(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    debug_print_str("ngx_http_proxy_hello called");

    ngx_http_core_loc_conf_t *clcf;

    clcf          = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_echo_handler;

    return NGX_CONF_OK;
}

static void *
ngx_http_echo_create_loc_conf(ngx_conf_t *cf)
{
    debug_print_str("ngx_http_echo_create_loc_conf called");

    ngx_http_echo_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_echo_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_str_null(&conf->message);

    return conf;
}

static char *
ngx_http_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    debug_print_str("ngx_http_echo_merge_loc_conf called");

    ngx_http_echo_loc_conf_t *prev = parent;
    ngx_http_echo_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->message, prev->message, "message from ngx_http_echo_merge_loc_conf");

    return NGX_CONF_OK;
}