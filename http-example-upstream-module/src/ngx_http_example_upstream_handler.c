
/*
 * Created by Wu Jian Ping on - 2022/01/18.
 */

#include "ddebug.h"

#include "ngx_http_example_upstream_handler.h"

static ngx_int_t ngx_http_example_upstream_create_request(ngx_http_request_t *r);
static ngx_int_t ngx_http_example_upstream_process_header(ngx_http_request_t *r);
static ngx_int_t ngx_http_example_upstream_filter_init(void *data);
static ngx_int_t ngx_http_example_upstream_filter(void *data, ssize_t bytes);
static ngx_int_t ngx_http_example_upstream_reinit_request(ngx_http_request_t *r);
static void ngx_http_example_upstream_abort_request(ngx_http_request_t *r);
static void ngx_http_example_upstream_finalize_request(ngx_http_request_t *r, ngx_int_t rc);

ngx_int_t
ngx_http_example_upstream_handler(ngx_http_request_t *r)
{
    ngx_write_stdout("ngx_http_example_upstream_handler\n");

    ngx_int_t rc;
    ngx_http_upstream_t *u;
    ngx_http_example_upstream_ctx_t *ctx;
    ngx_http_example_upstream_loc_conf_t *mlcf;

    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    /* IMPORTANT: setup content type */
    ngx_str_t content_type_header   = ngx_string("text/plain");
    r->headers_out.content_type     = content_type_header;
    r->headers_out.content_type_len = content_type_header.len;


    if (ngx_http_upstream_create(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    ngx_str_set(&u->schema, "redis://");

    u->output.tag = (ngx_buf_tag_t)&ngx_http_example_upstream_module;

    mlcf = ngx_http_get_module_loc_conf(r, ngx_http_example_upstream_module);

    u->conf = &mlcf->upstream;

    u->create_request   = ngx_http_example_upstream_create_request;
    u->reinit_request   = ngx_http_example_upstream_reinit_request;
    u->process_header   = ngx_http_example_upstream_process_header;
    u->abort_request    = ngx_http_example_upstream_abort_request;
    u->finalize_request = ngx_http_example_upstream_finalize_request;

    ctx = ngx_palloc(r->pool, sizeof(ngx_http_example_upstream_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->request = r;

    ngx_http_set_ctx(r, ctx, ngx_http_example_upstream_module);

    u->input_filter_init = ngx_http_example_upstream_filter_init;
    u->input_filter      = ngx_http_example_upstream_filter;
    u->input_filter_ctx  = ctx;

    r->main->count++;

    ngx_http_upstream_init(r);

    return NGX_DONE;
}

static ngx_int_t
ngx_http_example_upstream_create_request(ngx_http_request_t *r)
{
    ngx_write_stdout("ngx_http_example_upstream_create_request\n");

    ngx_buf_t *b;
    ngx_chain_t *cl;
    ngx_str_t command;

    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    // 先写死一条命令
    ngx_str_set(&command, "get foo\r\n");

    b->pos    = command.data;
    b->last   = command.data + command.len;
    b->memory = 1;

    cl->buf  = b;
    cl->next = NULL;

    r->upstream->request_bufs = cl;

    return NGX_OK;
}

static ngx_int_t
ngx_http_example_upstream_process_header(ngx_http_request_t *r)
{
    ngx_write_stdout("ngx_http_example_upstream_process_header\n");

    ngx_http_upstream_t *u;
    ngx_buf_t *b;
    u_char chr;
    ngx_str_t buf;
    u_char *p;

    u = r->upstream;
    b = &u->buffer;

    for (p = u->buffer.pos; p < u->buffer.last; p++) {
        if (*p == LF) {
            goto found;
        }
    }


    return NGX_AGAIN;

found:
    chr = *b->pos;

    ngx_str_t s;
    s.data = b->pos;
    s.len  = b->last - b->pos;
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "response: %V", &s);

    switch (chr) {
        case '+': // simple strings
        case '-': // errors
        case ':': // integers
        case '$': // bulk strings
        case '*': // array
            // ctx->filter = ngx_http_redis2_process_reply;
            break;

        default:
            buf.data = b->pos;
            buf.len  = b->last - b->pos;

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "redis sent invalid response: %V", &buf);

            return NGX_HTTP_UPSTREAM_INVALID_HEADER;
    }

    u->headers_in.status_n         = NGX_HTTP_OK; /* set http status */
    u->state->status               = NGX_HTTP_OK; /* set http status */
    u->headers_in.content_length_n = 9;

    return NGX_OK;
}

static ngx_int_t
ngx_http_example_upstream_filter_init(void *data)
{
    ngx_write_stdout("ngx_http_example_upstream_filter_init\n");

    ngx_http_example_upstream_ctx_t *ctx = data;

    ngx_http_upstream_t *u;

    u = ctx->request->upstream;

    u->length = 0;

    return NGX_OK;
}

static ngx_int_t
ngx_http_example_upstream_filter(void *data, ssize_t bytes)
{
    debug_print("ngx_http_example_upstream_filter, received size: %d\n", bytes);

    ngx_http_example_upstream_ctx_t *ctx = data;

    u_char *last;
    ngx_buf_t *b;
    ngx_chain_t *cl, **ll;
    ngx_http_upstream_t *u;

    int i;

    u = ctx->request->upstream;
    b = &u->buffer;

    for (cl = u->out_bufs, ll = &u->out_bufs, i = 0; cl; cl = cl->next, i++) {
        ll = &cl->next;
        debug_print("loop: %d", i);
    }

    cl = ngx_chain_get_free_buf(ctx->request->pool, &u->free_bufs);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf->flush  = 1;
    cl->buf->memory = 1;

    *ll = cl;

    last         = b->last;
    cl->buf->pos = last;
    b->last += bytes;
    cl->buf->last = b->last;
    cl->buf->tag  = u->output.tag;

    if (u->length == 0) {
        u->keepalive = 1;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_example_upstream_reinit_request(ngx_http_request_t *r)
{
    ngx_write_stdout("ngx_http_example_upstream_reinit_request\n");


    return NGX_OK;
}


static void
ngx_http_example_upstream_abort_request(ngx_http_request_t *r)
{
    ngx_write_stdout("ngx_http_example_upstream_abort_request\n");


    return;
}


static void
ngx_http_example_upstream_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_write_stdout("ngx_http_example_upstream_finalize_request\n");


    return;
}
