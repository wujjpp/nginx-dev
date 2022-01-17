
/*
 * Created by Wu Jian Ping on - 2022/01/17.
 */


#include "ngx_http_redis_module.h"

static ngx_int_t
ngx_http_redis_init(ngx_conf_t *cf);
static void *
ngx_http_redis_create_loc_conf(ngx_conf_t *cf);
static char *
ngx_http_redis_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t
ngx_http_redis_handler(ngx_http_request_t *r);


static ngx_command_t ngx_http_redis_commands[] = {
    { ngx_string("redis_pass"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
    ngx_null_command
};

static ngx_http_module_t ngx_http_redis_ctx = {
    NULL,
    ngx_http_redis_init,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_redis_create_loc_conf,
    ngx_http_redis_merge_loc_conf,
};


ngx_module_t ngx_http_redis_module = {
    NGX_MODULE_V1,
    &ngx_http_redis_ctx,     /* module context */
    ngx_http_redis_commands, /* module directives */
    NGX_HTTP_MODULE,         /* module type */
    NULL,                    /* init master */
    NULL,                    /* init module */
    NULL,                    /* init process */
    NULL,                    /* init thread */
    NULL,                    /* exit thread */
    NULL,                    /* exit process */
    NULL,                    /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_http_redis_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_redis_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_redis_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    ngx_str_null(&conf->redis_addr);

    return conf;
}

static char *
ngx_http_redis_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_redis_loc_conf_t *prev = parent;
    ngx_http_redis_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->redis_addr, prev->redis_addr, "127.0.0.1:6379");

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_redis_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    h    = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_redis_handler;

    return NGX_OK;
}


static ngx_int_t
ngx_http_redis_handler(ngx_http_request_t *r)
{
    ngx_http_redis_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_redis_module);

    if (conf == NULL)
        return NGX_HTTP_INTERNAL_SERVER_ERROR;

    ngx_log_error(NGX_LOG_NOTICE, r->connection->log, 0, "redis_addr: %s", conf->redis_addr.data);

    return NGX_HTTP_BAD_REQUEST;
}
