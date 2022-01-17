/**
 * Created by Wu Jian Ping on - 2022/01/14.
 */

#include "ngx_http_example_filter_module.h"

static ngx_int_t ngx_http_example_filter_init(ngx_conf_t *cf);

static ngx_int_t ngx_http_example_header_filter(ngx_http_request_t *r);
static ngx_int_t ngx_http_example_body_filter(ngx_http_request_t *r, ngx_chain_t *in);

static void *ngx_http_example_filter_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_example_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_command_t ngx_http_example_filter_commands[] = {

    { ngx_string("example_filter_enable_header_filter"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_filter_loc_conf_t, enable_header_filter),
      NULL },

    { ngx_string("example_filter_content_prefix"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_filter_loc_conf_t, prefix_message),
      NULL },

    { ngx_string("example_filter_content_suffix"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_filter_loc_conf_t, suffix_message),
      NULL },

    ngx_null_command
};


static ngx_http_module_t ngx_http_example_filter_ctx = {
    NULL,
    ngx_http_example_filter_init,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_example_filter_create_loc_conf,
    ngx_http_example_filter_merge_loc_conf,
};


ngx_module_t ngx_http_example_filter_module = {
    NGX_MODULE_V1,
    &ngx_http_example_filter_ctx,     /* module context */
    ngx_http_example_filter_commands, /* module directives */
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

static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt ngx_http_next_body_filter;

static ngx_int_t
ngx_http_example_filter_init(ngx_conf_t *cf)
{
    ngx_log_debug0(NGX_LOG_NOTICE, cf->pool->log, 0, "liftcycle: ngx_http_example_filter_init called");

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter  = ngx_http_example_body_filter;

    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter  = ngx_http_example_header_filter;

    return NGX_OK;
}

static void *
ngx_http_example_filter_create_loc_conf(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "liftcycle: ngx_http_example_filter_create_loc_conf called");

    ngx_http_example_filter_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_filter_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable_header_filter = NGX_CONF_UNSET;
    ngx_str_null(&conf->prefix_message);
    ngx_str_null(&conf->suffix_message);

    return conf;
}

static char *
ngx_http_example_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "liftcycle: ngx_http_example_filter_merge_loc_conf called");

    ngx_http_example_filter_loc_conf_t *prev = parent;
    ngx_http_example_filter_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->enable_header_filter, prev->enable_header_filter, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_example_header_filter(ngx_http_request_t *r)
{
    ngx_log_error(NGX_LOG_NOTICE, r->pool->log, 0, "liftcycle: ngx_http_example_header_filter called");

    ngx_table_elt_t *v;
    ngx_uint_t i;
    ngx_list_part_t *part;
    ngx_http_example_filter_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_example_filter_module);

    if (conf->enable_header_filter) {

        /* append custom header */
        v = ngx_list_push(&r->headers_out.headers);
        if (v == NULL) {
            return NGX_ERROR;
        }
        v->hash = 1;
        ngx_str_set(&v->key, "x-request-from-app-name");
        ngx_str_set(&v->value, "nodejs-qcc-backend-data");


        /* iterate over the headers_out and remove or modify particular header */
        ngx_str_t x_powered_by           = ngx_string("X-Powered-By");
        ngx_str_t x_request_from_cluster = ngx_string("x-request-from-cluster");
        part                             = &r->headers_out.headers.part;
        v                                = part->elts;

        for (i = 0; /* void */; i++) {
            if (i >= part->nelts) {
                if (part->next == NULL) {
                    break;
                }
                part = part->next;
                v    = part->elts;
                i    = 0;
            }

            /* print header */
            ngx_log_error(NGX_LOG_NOTICE, r->pool->log, 0, "key:%s, value:%s", v[i].key.data, v[i].value.data);

            /* remove x-powered-by */
            if (ngx_strcasecmp(x_powered_by.data, v[i].key.data) == 0) {
                v[i].hash = 0; /* remove header */
            }

            /* modify x-request-cluster */
            if (ngx_strcasecmp(x_request_from_cluster.data, v[i].key.data) == 0) {
                ngx_str_set(&v[i].value, "cluster-b-extended");
            }
        }
    }

    // setup content-length
    if (conf->prefix_message.len > 0) {
        r->headers_out.content_length_n = r->headers_out.content_length_n + conf->prefix_message.len;
    }
    if (conf->suffix_message.len > 0) {
        r->headers_out.content_length_n = r->headers_out.content_length_n + conf->suffix_message.len;
    }

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_example_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_log_error(NGX_LOG_NOTICE, r->connection->log, 0, "liftcycle: ngx_http_example_body_filter called");

    ngx_chain_t *chain_link = in;

    ngx_chain_t prefix_chain;
    ngx_chain_t suffix_chain;
    ngx_buf_t *prefix_buf;
    ngx_buf_t *suffix_buf;

    ngx_http_example_filter_loc_conf_t *conf;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_example_filter_module);

    if (conf->prefix_message.len > 0) {
        prefix_buf           = ngx_palloc(r->pool, sizeof(ngx_buf_t));
        prefix_buf->pos      = conf->prefix_message.data;
        prefix_buf->last     = conf->prefix_message.data + conf->prefix_message.len;
        prefix_buf->memory   = 1;
        prefix_buf->last_buf = 1;

        prefix_chain.buf  = prefix_buf;
        prefix_chain.next = in;

        chain_link = &prefix_chain;
    }

    if (conf->suffix_message.len > 0) {
        suffix_buf           = ngx_palloc(r->pool, sizeof(ngx_buf_t));
        suffix_buf->pos      = conf->suffix_message.data;
        suffix_buf->last     = conf->suffix_message.data + conf->suffix_message.len;
        suffix_buf->memory   = 1;
        suffix_buf->last_buf = 1;
        suffix_chain.buf     = suffix_buf;

        in->next          = &suffix_chain;
        suffix_chain.next = NULL;
    }

    return ngx_http_next_body_filter(r, chain_link);
}
