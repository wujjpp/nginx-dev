
/*
 * Created by Wu Jian Ping on - 2022/01/13.
 */


#include "ngx_http_example_handler_module.h"
#include "ngx_http_example_handler_handler.h"

static void *ngx_http_example_handle_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_example_handle_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static char *ngx_http_example_handler_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_example_handler_enable_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_example_handle_commands[] = {

    {
        ngx_string("example_handler_response"), /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,    /* 允许的位置和参数控制 */
        ngx_http_example_handler_response,      /* 设置函数，这边使用了自定函数 */
        NGX_HTTP_LOC_CONF_OFFSET,               /* 使用自定设置函数，这个参数就没什么意义了 */
        0,                                      /* 使用自定设置函数，这个参数就没什么意义了 */
        NULL                                    /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    {
        ngx_string("example_handler_set_message"),              /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,                     /* 允许的位置和参数控制 */
        ngx_conf_set_str_slot,                                  /* 设置函数 */
        NGX_HTTP_LOC_CONF_OFFSET,                               /* 指定保存到哪个配置(main,server,location)上 */
        offsetof(ngx_http_example_handler_loc_conf_t, message), /* 将值保存到哪个属性上 */
        NULL                                                    /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    {
        ngx_string("example_handler_enable_counter"),           /* 指令名称 */
        NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,                      /* 允许的位置和参数控制 */
        ngx_http_example_handler_enable_counter,                /* 设置函数，这边使用了自定函数，注意：假如自定义函数里面使用 ngx_conf_set_xxx_slot 来设置属性的话，下面的offset必须要填，ngx_conf_set_xxx_slot函数内部通过offset来判定需要初始化哪个个数据 */
        NGX_HTTP_LOC_CONF_OFFSET,                               /* 指定保存到哪个配置(main,server,location)上 */
        offsetof(ngx_http_example_handler_loc_conf_t, counter), /* 将值保存到哪个属性上 */
        NULL                                                    /* 一般为NULL, 但是可以参考ngx_http_memcached模块 */
    },

    ngx_null_command /* 必须要以ngx_null_command作为数组的最后一个元素 */
};


static ngx_http_module_t ngx_http_example_handle_ctx = {
    NULL,                                    /* pre configuration */
    NULL,                                    /* post configuration */
    NULL,                                    /* create main configuration */
    NULL,                                    /* merge main configuration */
    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */
    ngx_http_example_handle_create_loc_conf, /* create location configuration */
    ngx_http_example_handle_merge_loc_conf   /* merge location configuration */
};

ngx_module_t ngx_http_example_handler_module = {
    NGX_MODULE_V1,
    &ngx_http_example_handle_ctx,     /* module context */
    ngx_http_example_handle_commands, /* module directives */
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

static char *
ngx_http_example_handler_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "lifecycle: ngx_http_example_handler_response called");

    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    // 设置handler
    clcf->handler = ngx_http_example_handle_handler;

    /*
    cf: 该参数里面保存从配置文件读取到的原始字符串以及相关的一些信息。
        特别注意的是这个参数的args字段是一个ngx_str_t类型的数组，该数组的首个元素是这个配置指令本身，
        第二个元素是指令的第一个参数，第三个元素是第二个参数，依次类推。
        注意：在postconfiguration中对应值不一样
    */
    ngx_str_t *value, *command_name;

    value        = cf->args->elts;
    command_name = &value[0];

    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "directive: %s", command_name->data);

    return NGX_CONF_OK;
}

static char *
ngx_http_example_handler_enable_counter(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char *rv = NULL;

    /* 通过这种方式设置模块配置参数，必须在定义指令时，明确offset, 可参考ngx_conf_file.c#L1034-1038 */
    rv = ngx_conf_set_flag_slot(cf, cmd, conf);

    return rv;
}

static void *
ngx_http_example_handle_create_loc_conf(ngx_conf_t *cf)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "lifecycle: ngx_http_example_handle_create_loc_conf called");

    ngx_http_example_handler_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_example_handler_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_str_null(&conf->message);

    conf->counter = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_example_handle_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_log_error(NGX_LOG_NOTICE, cf->log, 0, "lifecycle: ngx_http_example_handle_merge_loc_conf called");

    ngx_http_example_handler_loc_conf_t *prev = parent;
    ngx_http_example_handler_loc_conf_t *conf = child;

    ngx_conf_merge_str_value(conf->message, prev->message, "message from ngx_http_example_handle_merge_loc_conf");
    ngx_conf_merge_off_value(conf->counter, prev->counter, 0);

    return NGX_CONF_OK;
}