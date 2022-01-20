
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

/*
 * 和普通的handler差不多，只是handler不再处理内容输出，改成由upstream来完成
 * 主要通过 ngx_http_upstream_create 创建 upstream, 通过查看源码可以看出，这条函数不是实际意义上的创建连接，仅仅是创建了ngx_http_upstream_t结构体
 * 设置将upstream的各个回调函数，最后通过调用 ngx_http_upstream_init 完成一系列后续操作
 */
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


    /* create ngx_http_upstream_t */
    if (ngx_http_upstream_create(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    ngx_str_set(&u->schema, "redis://");

    u->output.tag = (ngx_buf_tag_t)&ngx_http_example_upstream_module;

    mlcf = ngx_http_get_module_loc_conf(r, ngx_http_example_upstream_module);

    u->conf = &mlcf->upstream;

    /* create_request crafts a request buffer (or chain of them) to be sent to the upstream */
    u->create_request = ngx_http_example_upstream_create_request;
    /* reinit_request is called if the connection to the back-end is reset (just before create_request is called for the second time) */
    u->reinit_request = ngx_http_example_upstream_reinit_request;
    /* process_header processes the first bit of the upstream’s response, and usually saves a pointer to the upstream’s "payload" */
    u->process_header = ngx_http_example_upstream_process_header;
    /* abort_request is called if the client aborts the request */
    u->abort_request = ngx_http_example_upstream_abort_request;
    /* finalize_request is called when Nginx is finished reading from the upstream */
    u->finalize_request = ngx_http_example_upstream_finalize_request;

    ctx = ngx_palloc(r->pool, sizeof(ngx_http_example_upstream_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ctx->request = r;

    /*
     * 通过 ngx_http_set_ctx 函数将初始化好的结构体ctx, 设置在上下文中
     * ngx_http_example_upstream_filter_init 和 ngx_http_example_upstream_filter函数在被调用时，第一个参数就是该对象
     */
    ngx_http_set_ctx(r, ctx, ngx_http_example_upstream_module);

    /* adjust content length, you can refer ngx_http_memcached module */
    u->input_filter_init = ngx_http_example_upstream_filter_init;
    /* input_filter is a body filter that can be called on the response body (e.g., to remove a trailer) */
    u->input_filter     = ngx_http_example_upstream_filter;
    u->input_filter_ctx = ctx;

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
    ngx_http_variable_value_t *vv;
    ngx_http_example_upstream_loc_conf_t *mlcf;

    mlcf = ngx_http_get_module_loc_conf(r, ngx_http_example_upstream_module);

    // 根据变量index, 获取具体变量结构体
    vv = ngx_http_get_indexed_variable(r, mlcf->index);

    if (vv == NULL || vv->not_found || vv->len == 0) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "the \"$redis_command\" variable is not set");
        return NGX_ERROR;
    }


    b = ngx_calloc_buf(r->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    b->pos    = vv->data;
    b->last   = vv->data + vv->len;
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

    /*
     * 假如在已接收到的数据中不存在LF,我们认为redis尚未返回一行完成数据，
     * NOTE: 这样描述有点不严谨，但是也找不到好的方式描述，先这么干，
     * 可以去了解一下redis response data协议，不算复杂
     */
    return NGX_AGAIN;

found:
    chr = *b->pos;

    ngx_str_t s;
    s.data = b->pos;
    s.len  = b->last - b->pos;
    ngx_log_error(NGX_LOG_NOTICE, r->connection->log, 0, "response: %V", &s);

    switch (chr) {
        case '+': /* simple string
                   * +OK
                   */

        case '-': /* error
                   * -ERR wrong number of arguments for 'set' command
                   */

        case ':': /* integer
                   * :-1
                   */

        case '$': /* bulk string
                   * $3
                   * bar
                   */

        case '*': /* array
                     *2
                     $1
                     0
                     *3
                     $9
                     test-hash
                     $3
                     foo
                     $5
                     mykey
                   */
            break;

        default:
            buf.data = b->pos;
            buf.len  = b->last - b->pos;

            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "redis sent invalid response: %V", &buf);

            return NGX_HTTP_UPSTREAM_INVALID_HEADER;
    }

    u->headers_in.status_n         = NGX_HTTP_OK;      /* set http status */
    u->state->status               = NGX_HTTP_OK;      /* set http status */
    u->headers_in.content_length_n = b->last - b->pos; /* 设置content length */
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
