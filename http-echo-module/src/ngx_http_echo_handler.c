/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#include "ddebug.h"

#include "ngx_http_echo_handler.h"

static int ngx_hello_visited_times = 0;

ngx_int_t
ngx_http_echo_handler(ngx_http_request_t *r)
{
    ngx_table_elt_t *h;
    ngx_int_t rc;
    ngx_http_echo_loc_conf_t *conf;
    ngx_chain_t out;
    ngx_buf_t *buf;

    ngx_uint_t content_length     = 0;
    u_char response_message[1024] = { 0 };

    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* ======================= 测试代码开始 =======================
     * 下面的代码只是把请求的http协议的路径转化成一个文件系统的路径
     * 代码来自 nginx/src/http/modules/ngx_http_static_module.c#L77-83
     */
    size_t root;
    ngx_str_t path;
    u_char *last;
    last = ngx_http_map_uri_to_path(r, &path, &root, 0); /* /hello 转换成 /usr/local/nginx/html/hello */
    if (last == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    path.len = last - path.data;
    debug_print_ngx_str_t("uri -> path: %s", &path);
    /* ======================= 测试代码开始 ======================= */

    /* 不需要 request body */
    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    conf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);


    if (conf->message.len == 0) {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "message is empty!");
        return NGX_DECLINED;
    }

    if (conf->counter == NGX_CONF_UNSET || conf->counter == 0) {
        ngx_sprintf(response_message, "hi %s", conf->message.data);
    }
    else {
        ngx_sprintf(response_message, "hi %s, you have visited %d times", conf->message.data, ++ngx_hello_visited_times);
    }

    content_length = ngx_strlen(response_message);

    /* we response to 'GET' and 'HEAD' requests only */
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    /* discard request body, since we don't need it here */
    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    /* send the header only, if the request type is http 'HEAD' */
    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status           = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;

        return ngx_http_send_header(r);
    }

    buf = ngx_palloc(r->pool, sizeof(ngx_buf_t));
    if (buf == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    /* 这里比较牛逼，直接用内存地址 */
    buf->pos      = response_message;
    buf->last     = response_message + content_length;
    buf->memory   = 1; /* this buffer is in memory */
    buf->last_buf = 1; /* this is the last buffer in the buffer chain */

    out.buf  = buf;
    out.next = NULL;


    ngx_str_set(&r->headers_out.content_type, "text/html"); /* setup content type */
    r->headers_out.status           = NGX_HTTP_OK;          /* setup status code */
    r->headers_out.content_length_n = content_length;       /* setup content length */

    h = ngx_list_push(&r->headers_out.headers); /* append custom header */
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->hash = 1;
    ngx_str_set(&h->key, "x-request-from-app-name");
    ngx_str_set(&h->value, "just test");

    /* send the headers of your response */
    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    /* send the buffer chain of your response */
    return ngx_http_output_filter(r, &out);
}

ngx_int_t
ngx_http_echo_access_handler(ngx_http_request_t *r)
{
    ngx_http_echo_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);
    if (conf->allow)
        return NGX_OK;

    return NGX_HTTP_FORBIDDEN;
}
