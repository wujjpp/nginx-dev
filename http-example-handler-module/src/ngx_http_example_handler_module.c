
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#include "ddebug.h"

#include "ngx_http_example_handler_handler.h"
#include "ngx_http_example_handler_module.h"

static void *ngx_http_example_handle_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_example_handle_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static char *ngx_http_example_handler_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_example_handler_enable_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_example_handle_commands[] = {

    { ngx_string("example_handler_response"),
      NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
      ngx_http_example_handler_response,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("example_handler_set_message"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_handler_loc_conf_t, message),
      NULL },

    { ngx_string("example_handler_enable_counter"),
      NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
      ngx_http_example_handler_enable_counter,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_handler_loc_conf_t, counter),
      NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_example_handle_ctx = {
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_example_handle_create_loc_conf,
    ngx_http_example_handle_merge_loc_conf
};

ngx_module_t ngx_http_example_handler_module = {
    NGX_MODULE_V1,
    &ngx_http_example_handle_ctx,     /* module context */
    ngx_http_example_handle_commands, /* module directives */
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

static char *
ngx_http_example_handler_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    debug_print_str("lifecycle: ngx_http_example_handler_response called");

    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    // 设置handler
    clcf->handler = ngx_http_example_handle_handler;

    /*
    cf: 该参数里面保存从配置文件读取到的原始字符串以及相关的一些信息。
        特别注意的是这个参数的args字段是一个ngx_str_t类型的数组，该数组的首个元素是这个配置指令本身，
        第二个元素是指令的第一个参数，第三个元素是第二个参数，依次类推。
    */
    ngx_str_t *value, *command_name;

    value        = cf->args->elts;
    command_name = &value[0];

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "command name: %s", command_name->data);

    return NGX_CONF_OK;
}

static char *
ngx_http_example_handler_enable_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_example_handler_loc_conf_t *local_conf = conf;

    char *rv = NULL;

    rv = ngx_conf_set_flag_slot(cf, cmd, conf);

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_counter:%d", local_conf->counter);

    return rv;
}

static void *
ngx_http_example_handle_create_loc_conf(ngx_conf_t *cf)
{
    debug_print_str("lifecycle: ngx_http_example_handle_create_loc_conf called");

    ngx_http_example_handler_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_handler_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_str_null(&conf->message);

    conf->counter = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_example_handle_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    debug_print_str("lifecycle: ngx_http_example_handle_merge_loc_conf called");

    ngx_http_example_handler_loc_conf_t *prev = parent;
    ngx_http_example_handler_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->message, prev->message, "message from ngx_http_example_handle_merge_loc_conf");
    ngx_conf_merge_off_value(conf->counter, prev->counter, 0);

    return NGX_CONF_OK;
}