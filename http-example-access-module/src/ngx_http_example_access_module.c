
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */


#include "ngx_http_example_access_module.h"


static ngx_int_t ngx_http_example_access_init(ngx_conf_t *cf);
static void *ngx_http_example_access_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_example_access_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t ngx_http_example_access_handler(ngx_http_request_t *r);

/* 定义模块指令 */
static ngx_command_t ngx_http_example_access_commands[] = {

    {
        ngx_string("example_access_deny"),                  /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,                  /* 允许的位置和参数控制 */
        ngx_conf_set_flag_slot,                             /* 设置函数 */
        NGX_HTTP_LOC_CONF_OFFSET,                           /* 这边取决于 */
        offsetof(ngx_http_example_access_loc_conf_t, deny), /* 将值保存到哪个属性上 */
        NULL                                                /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    ngx_null_command /* 必须要以ngx_null_command作为数组的最后一个元素 */
};

/* 创建、合并3种配置文件（main, server, location）钩子函数 */
static ngx_http_module_t ngx_http_example_access_ctx = {
    NULL,                                    /* pre configuration */
    ngx_http_example_access_init,            /* post configuration */
    NULL,                                    /* create main configuration */
    NULL,                                    /* merge main configuration */
    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */
    ngx_http_example_access_create_loc_conf, /* create location configuration */
    ngx_http_example_access_merge_loc_conf   /* merge location configuration */
};

/* 定义模块, 注意：这边不能用static */
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
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "lifecycle: ngx_http_example_access_create_loc_conf called");

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
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "lifecycle: ngx_http_example_access_merge_loc_conf called");

    ngx_http_example_access_loc_conf_t *prev = parent;
    ngx_http_example_access_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->deny, prev->deny, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_example_access_init(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "lifecycle: ngx_http_example_access_init called");

    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    /* 将handler设置在NGX_HTTP_ACCESS_PHASE阶段执行 */
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);

    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_example_access_handler;

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
