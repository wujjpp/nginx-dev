
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */


#include "ngx_http_example_access_module.h"


static ngx_int_t ngx_http_example_access_init(ngx_conf_t *cf);

static void *ngx_http_example_access_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_example_access_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t ngx_http_example_access_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_example_access_commands[] = {

    { ngx_string("example_access_deny"),
      NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_access_loc_conf_t, deny),
      NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_example_access_ctx = {
    NULL,
    ngx_http_example_access_init,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_example_access_create_loc_conf,
    ngx_http_example_access_merge_loc_conf
};

ngx_module_t ngx_http_example_access_module = {
    NGX_MODULE_V1,
    &ngx_http_example_access_ctx,     /* module context */
    ngx_http_example_access_commands, /* module directives */
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

static void *
ngx_http_example_access_create_loc_conf(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "lifecycle: ngx_http_example_access_create_loc_conf called");

    ngx_http_example_access_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_access_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->deny = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_example_access_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "lifecycle: ngx_http_example_access_merge_loc_conf called");

    ngx_http_example_access_loc_conf_t *prev = parent;
    ngx_http_example_access_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->deny, prev->deny, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_example_access_init(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "lifecycle: ngx_http_example_access_init called");

    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);

    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_example_access_handler;

    /*
     * cf: 该参数里面保存从配置文件读取到的原始字符串以及相关的一些信息。
     *     特别注意的是这个参数的args字段是一个ngx_str_t类型的数组，该数组的首个元素是这个配置指令本身，
     *     第二个元素是指令的第一个参数，第三个元素是第二个参数，依次类推。
     */

    ngx_str_t *value, *command_name;

    value        = cf->args->elts;
    command_name = &value[0];

    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "command name: %s", command_name->data);

    return NGX_OK;
}

static ngx_int_t
ngx_http_example_access_handler(ngx_http_request_t *r)
{
    ngx_http_example_access_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_example_access_module);
    if (!conf->deny)
        return NGX_OK;

    return NGX_HTTP_FORBIDDEN;
}
