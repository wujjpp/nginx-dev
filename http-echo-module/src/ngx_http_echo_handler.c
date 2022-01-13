/**
 * Created by Wu Jian Ping on - 2022/01/13.
 */

#include "ddebug.h"

#include "ngx_http_echo_handler.h"

ngx_int_t
ngx_http_echo_handler(ngx_http_request_t *r)
{
    ngx_table_elt_t *h;
    ngx_int_t rc;
    ngx_http_echo_loc_conf_t *conf;
    ngx_str_t *response;
    ngx_buf_t *buf0;
    ngx_buf_t *buf1;
    ngx_chain_t *out0;
    ngx_chain_t *out1;

    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    conf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);

    response = ngx_palloc(r->pool, sizeof(ngx_str_t));

    if (conf->message.len > 0) {
        response->len  = conf->message.len;
        response->data = conf->message.data;
    }
    else {
        response->len  = sizeof("nginx") - 1;
        response->data = (u_char *)"nginx";
    }

    debug_print_ngx_str_t(response);

    ngx_str_t body_prefix = ngx_string("Hi, ");

    buf0 = ngx_create_temp_buf(r->pool, body_prefix.len);
    if (buf0 == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_memcpy(buf0->pos, body_prefix.data, body_prefix.len);
    buf0->last     = buf0->pos + body_prefix.len;
    buf0->last_buf = 0;

    ngx_str_t type                  = ngx_string("text/plain");
    r->headers_out.status           = NGX_HTTP_OK;
    r->headers_out.content_length_n = response->len + body_prefix.len;
    r->headers_out.content_type     = type;

    /* 添加一个自定义头 */
    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->hash = 1;
    ngx_str_set(&h->key, "x-request-from-app-name");
    ngx_str_set(&h->value, "just test");

    // 发送header
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    // 初始化buffer1
    buf1 = ngx_create_temp_buf(r->pool, response->len);
    if (buf1 == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_memcpy(buf1->pos, response->data, response->len);
    buf1->last     = buf1->pos + response->len;
    buf1->last_buf = 1;

    out0 = ngx_alloc_chain_link(r->pool);
    if (out0 == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out1 = ngx_alloc_chain_link(r->pool);
    if (out1 == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    out0->buf  = buf0;
    out0->next = out1;

    out1->buf  = buf1;
    out1->next = NULL;

    return ngx_http_output_filter(r, out0);
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
