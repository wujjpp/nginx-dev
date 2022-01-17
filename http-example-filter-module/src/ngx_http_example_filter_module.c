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

    { ngx_string("example_filter"),
      NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_example_filter_loc_conf_t, enable),
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

    conf->enable = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_example_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->pool->log, 0, "liftcycle: ngx_http_example_filter_merge_loc_conf called");

    ngx_http_example_filter_loc_conf_t *prev = parent;
    ngx_http_example_filter_loc_conf_t *conf = child;

    ngx_conf_merge_off_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_example_header_filter(ngx_http_request_t *r)
{
    ngx_log_error(NGX_LOG_NOTICE, r->pool->log, 0, "liftcycle: ngx_http_example_header_filter called");

    ngx_table_elt_t *v;
    ngx_uint_t i;
    ngx_list_part_t *part;

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

    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_example_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_log_error(NGX_LOG_NOTICE, r->connection->log, 0, "liftcycle: ngx_http_example_body_filter called");

    return ngx_http_next_body_filter(r, in);
}
