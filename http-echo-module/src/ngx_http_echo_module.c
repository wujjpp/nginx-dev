/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#include "ddebug.h"

#include "ngx_http_echo_handler.h"
#include "ngx_http_echo_module.h"

static ngx_int_t ngx_http_echo_pre_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_echo_post_conf(ngx_conf_t *cf);
static void *ngx_http_echo_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static char *ngx_http_proxy_hello(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_echo_commands[] = {

    { ngx_string("hello_proxy"),
      NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
      ngx_http_proxy_hello,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("hello_string"),
      NGX_HTTP_LOC_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_MAIN_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_echo_loc_conf_t, message),
      NULL },

    { ngx_string("hello_allow"),
      NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_echo_loc_conf_t, allow),
      NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_echo_ctx = {
    ngx_http_echo_pre_conf,
    ngx_http_echo_post_conf,

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

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    // 设置handler
    clcf->handler = ngx_http_echo_handler;

    /*
    cf: 该参数里面保存从配置文件读取到的原始字符串以及相关的一些信息。
        特别注意的是这个参数的args字段是一个ngx_str_t类型的数组，该数组的首个元素是这个配置指令本身，
        第二个元素是指令的第一个参数，第三个元素是第二个参数，依次类推。
    */
    ngx_str_t *value, *command_name;

    value        = cf->args->elts;
    command_name = &value[0];

    debug_print_ngx_str_t(command_name);

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

    conf->allow = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    debug_print_str("ngx_http_echo_merge_loc_conf called");

    ngx_http_echo_loc_conf_t *prev = parent;
    ngx_http_echo_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->message, prev->message, "message from ngx_http_echo_merge_loc_conf");
    ngx_conf_merge_off_value(conf->allow, prev->allow, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_echo_pre_conf(ngx_conf_t *cf)
{
    debug_print_str("ngx_http_echo_pre_conf");
    return NGX_OK;
}

static ngx_int_t
ngx_http_echo_post_conf(ngx_conf_t *cf)
{
    debug_print_str("ngx_http_echo_post_conf - 也可以在这里设置handler，假如不需要参数的话");

    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);

    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_echo_access_handler;

    // /*
    // cf: 该参数里面保存从配置文件读取到的原始字符串以及相关的一些信息。
    //     特别注意的是这个参数的args字段是一个ngx_str_t类型的数组，该数组的首个元素是这个配置指令本身，
    //     第二个元素是指令的第一个参数，第三个元素是第二个参数，依次类推。
    // */
    // ngx_str_t *value, *command_name;

    // value        = cf->args->elts;
    // command_name = &value[0];

    // debug_print_ngx_str_t(command_name);

    return NGX_OK;
}