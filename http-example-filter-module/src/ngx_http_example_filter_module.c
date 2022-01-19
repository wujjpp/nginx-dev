
/*
 * Created by Wu Jian Ping on - 2022/01/14.
 */

#include "ngx_http_example_filter_module.h"

static ngx_int_t ngx_http_example_filter_init(ngx_conf_t *cf);

static ngx_int_t ngx_http_example_header_filter(ngx_http_request_t *r);
static ngx_int_t ngx_http_example_body_filter(ngx_http_request_t *r, ngx_chain_t *in);

static void *ngx_http_example_filter_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_example_filter_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_command_t ngx_http_example_filter_commands[] = {

    {
        ngx_string("example_filter_enable_header_filter"),                  /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                                 /* 允许的位置和参数控制 */
        ngx_conf_set_flag_slot,                                             /* 设置函数 */
        NGX_HTTP_LOC_CONF_OFFSET,                                           /* 指定保存到哪个配置(main,server,location)上 */
        offsetof(ngx_http_example_filter_loc_conf_t, enable_header_filter), /* 将值保存到哪个属性上 */
        NULL                                                                /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    {
        ngx_string("example_filter_content_prefix"),                  /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                           /* 允许的位置和参数控制 */
        ngx_conf_set_str_slot,                                        /* 设置函数 */
        NGX_HTTP_LOC_CONF_OFFSET,                                     /* 指定保存到哪个配置(main,server,location)上 */
        offsetof(ngx_http_example_filter_loc_conf_t, prefix_message), /* 将值保存到哪个属性上 */
        NULL                                                          /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    {
        ngx_string("example_filter_content_suffix"),                  /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                           /* 允许的位置和参数控制 */
        ngx_conf_set_str_slot,                                        /* 设置函数 */
        NGX_HTTP_LOC_CONF_OFFSET,                                     /* 指定保存到哪个配置(main,server,location)上 */
        offsetof(ngx_http_example_filter_loc_conf_t, suffix_message), /* 将值保存到哪个属性上 */
        NULL                                                          /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    ngx_null_command /* 必须要以ngx_null_command作为数组的最后一个元素 */
};


static ngx_http_module_t ngx_http_example_filter_ctx = {
    NULL,                                    /* pre configuration */
    ngx_http_example_filter_init,            /* post configuration */
    NULL,                                    /* create main configuration */
    NULL,                                    /* merge main configuration */
    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */
    ngx_http_example_filter_create_loc_conf, /* create location configuration */
    ngx_http_example_filter_merge_loc_conf,  /* merge location configuration */
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

/* 下面两个变量用于将当前filter加入到nginx filter链中 */
static ngx_http_output_header_filter_pt ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt ngx_http_next_body_filter;

static ngx_int_t
ngx_http_example_filter_init(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "liftcycle: ngx_http_example_filter_init called");

    /* 逻辑说明：
     *   ngx_http_top_body_filter 和 ngx_http_top_header_filter 这两个变量是系统全局变量，
     *   在配置完该模块之后，通过将 ngx_http_top_header_filter 保存在 ngx_http_next_header_filter，
     *   将 ngx_http_top_body_filter 保存在ngx_http_next_body_filter变量上，
     *   将原 ngx_http_top_header_filter 设置成该模块的 ngx_http_example_header_filter，
     *   将原 ngx_http_top_body_filter 设置成该模块的 ngx_http_example_body_filter，
     *   完成调用链插入一个函数指针，
     *   可以用链表插入节点的模式来理解。
     */
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter  = ngx_http_example_header_filter;

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter  = ngx_http_example_body_filter;


    return NGX_OK;
}

static void *
ngx_http_example_filter_create_loc_conf(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "liftcycle: ngx_http_example_filter_create_loc_conf called");

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
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "liftcycle: ngx_http_example_filter_merge_loc_conf called");

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
    ngx_log_error(NGX_LOG_NOTICE, r->pool->log, 0, "liftcycle: ngx_http_example_body_filter called");

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
