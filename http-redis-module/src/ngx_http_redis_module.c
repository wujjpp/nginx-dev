
/*
 * Created by Wu Jian Ping on - 2022/01/17.
 */


#include "ngx_http_redis_module.h"
#include "ngx_http_redis_handler.h"


static void *
ngx_http_redis_create_loc_conf(ngx_conf_t *cf);
static char *
ngx_http_redis_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static char *
ngx_http_redis_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_redis_commands[] = {
    { ngx_string("redis_pass"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_http_redis_pass,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
    ngx_null_command
};

static ngx_http_module_t ngx_http_redis_ctx = {
    NULL,
    NULL,

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

    conf->upstream.local                 = NGX_CONF_UNSET_PTR;
    conf->upstream.socket_keepalive      = NGX_CONF_UNSET;
    conf->upstream.next_upstream_tries   = NGX_CONF_UNSET_UINT;
    conf->upstream.connect_timeout       = NGX_CONF_UNSET_MSEC;
    conf->upstream.send_timeout          = NGX_CONF_UNSET_MSEC;
    conf->upstream.read_timeout          = NGX_CONF_UNSET_MSEC;
    conf->upstream.next_upstream_timeout = NGX_CONF_UNSET_MSEC;

    conf->upstream.buffer_size = NGX_CONF_UNSET_SIZE;

    /* the hardcoded values */
    conf->upstream.cyclic_temp_file     = 0;
    conf->upstream.buffering            = 0;
    conf->upstream.ignore_client_abort  = 0;
    conf->upstream.send_lowat           = 0;
    conf->upstream.bufs.num             = 0;
    conf->upstream.busy_buffers_size    = 0;
    conf->upstream.max_temp_file_size   = 0;
    conf->upstream.temp_file_write_size = 0;
    conf->upstream.intercept_errors     = 1;
    conf->upstream.intercept_404        = 1;
    conf->upstream.pass_request_headers = 0;
    conf->upstream.pass_request_body    = 0;
    conf->upstream.force_ranges         = 1;

    return conf;
}

static char *
ngx_http_redis_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_redis_loc_conf_t *prev = parent;
    ngx_http_redis_loc_conf_t *conf = child;

    ngx_conf_merge_ptr_value(conf->upstream.local,
                             prev->upstream.local, NULL);

    ngx_conf_merge_value(conf->upstream.socket_keepalive,
                         prev->upstream.socket_keepalive, 0);

    ngx_conf_merge_uint_value(conf->upstream.next_upstream_tries,
                              prev->upstream.next_upstream_tries, 0);

    ngx_conf_merge_msec_value(conf->upstream.connect_timeout,
                              prev->upstream.connect_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.send_timeout,
                              prev->upstream.send_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.read_timeout,
                              prev->upstream.read_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.next_upstream_timeout,
                              prev->upstream.next_upstream_timeout, 0);

    ngx_conf_merge_size_value(conf->upstream.buffer_size,
                              prev->upstream.buffer_size,
                              (size_t)ngx_pagesize);

    ngx_conf_merge_bitmask_value(conf->upstream.next_upstream,
                                 prev->upstream.next_upstream,
                                 (NGX_CONF_BITMASK_SET
                                  | NGX_HTTP_UPSTREAM_FT_ERROR
                                  | NGX_HTTP_UPSTREAM_FT_TIMEOUT));

    if (conf->upstream.next_upstream & NGX_HTTP_UPSTREAM_FT_OFF) {
        conf->upstream.next_upstream = NGX_CONF_BITMASK_SET
            | NGX_HTTP_UPSTREAM_FT_OFF;
    }

    if (conf->upstream.upstream == NULL) {
        conf->upstream.upstream = prev->upstream.upstream;
    }

    return NGX_CONF_OK;
}

static char *
ngx_http_redis_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_redis_loc_conf_t *mlcf = conf;

    ngx_str_t *value;
    ngx_url_t u;
    ngx_http_core_loc_conf_t *clcf;

    if (mlcf->upstream.upstream) {
        return "is duplicate";
    }

    value = cf->args->elts;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url        = value[1];
    u.no_resolve = 1;

    mlcf->upstream.upstream = ngx_http_upstream_add(cf, &u, 0);
    if (mlcf->upstream.upstream == NULL) {
        return NGX_CONF_ERROR;
    }

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    clcf->handler = ngx_http_redis_handler;

    if (clcf->name.len && clcf->name.data[clcf->name.len - 1] == '/') {
        clcf->auto_redirect = 1;
    }

    return NGX_CONF_OK;
}