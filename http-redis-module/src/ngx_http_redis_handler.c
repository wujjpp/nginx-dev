
/*
 * Created by Wu Jian Ping on - 2022/01/18.
 */


#include "ngx_http_redis_handler.h"


static ngx_int_t ngx_http_redis_create_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis_process_header(ngx_http_request_t *r);
static ngx_int_t ngx_http_redis_filter_init(void *data);
static ngx_int_t ngx_http_memcached_filter(void *data, ssize_t bytes);
static ngx_int_t ngx_http_redis_reinit_request(ngx_http_request_t *r);
static void ngx_http_redis_abort_request(ngx_http_request_t *r);
static void ngx_http_redis_finalize_request(ngx_http_request_t *r, ngx_int_t rc);


ngx_int_t
ngx_http_redis_handler(ngx_http_request_t *r)
{
    ngx_int_t rc;
    ngx_http_upstream_t *u;
    ngx_http_redis_ctx_t *ctx;
    ngx_http_redis_loc_conf_t *mlcf;

    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    if (ngx_http_set_content_type(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ngx_http_upstream_create(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    ngx_str_set(&u->schema, "redis://");
    u->output.tag = (ngx_buf_tag_t)&ngx_http_redis_module;

    mlcf = ngx_http_get_module_loc_conf(r, ngx_http_redis_module);

    u->conf = &mlcf->upstream;

    u->create_request   = ngx_http_redis_create_request;
    u->reinit_request   = ngx_http_redis_reinit_request;
    u->process_header   = ngx_http_redis_process_header;
    u->abort_request    = ngx_http_redis_abort_request;
    u->finalize_request = ngx_http_redis_finalize_request;

    ctx = ngx_palloc(r->pool, sizeof(ngx_http_redis_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->request = r;

    ngx_http_set_ctx(r, ctx, ngx_http_redis_module);

    u->input_filter_init = ngx_http_redis_filter_init;
    u->input_filter      = ngx_http_memcached_filter;
    u->input_filter_ctx  = ctx;

    r->main->count++;

    ngx_http_upstream_init(r);

    return NGX_DONE;
}

static ngx_int_t
ngx_http_redis_create_request(ngx_http_request_t *r)
{
    return NGX_OK;
}

static ngx_int_t
ngx_http_redis_process_header(ngx_http_request_t *r)
{
    return NGX_OK;
}

static ngx_int_t
ngx_http_redis_filter_init(void *data)
{
    return NGX_OK;
}

static ngx_int_t
ngx_http_memcached_filter(void *data, ssize_t bytes)
{
    return NGX_OK;
}


static ngx_int_t
ngx_http_redis_reinit_request(ngx_http_request_t *r)
{
    return NGX_OK;
}


static void
ngx_http_redis_abort_request(ngx_http_request_t *r)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "abort http redis request");
    return;
}


static void
ngx_http_redis_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "finalize http redis request");
    return;
}
