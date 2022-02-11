/*
 * Created by Wu Jian Ping on - 2022/02/11.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    in_addr_t mask;
    in_addr_t addr;
    ngx_uint_t deny; /* unsigned  deny:1; */
} ngx_http_access_rule_t;

typedef struct {
    ngx_array_t *rules; /* array of ngx_http_access_rule_t */
} ngx_http_access_loc_conf_t;


static ngx_int_t ngx_http_access_handler(ngx_http_request_t *r);
static ngx_int_t ngx_http_access_inet(ngx_http_request_t *r,
                                      ngx_http_access_loc_conf_t *alcf, in_addr_t addr);


static ngx_int_t ngx_http_access_found(ngx_http_request_t *r, ngx_uint_t deny);
static char *ngx_http_access_rule(ngx_conf_t *cf, ngx_command_t *cmd,
                                  void *conf);
static void *ngx_http_access_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_access_merge_loc_conf(ngx_conf_t *cf,
                                            void *parent, void *child);
static ngx_int_t ngx_http_access_init(ngx_conf_t *cf);

static ngx_command_t ngx_http_access2_commands[] = {

    { ngx_string("allow_x_request_client_real_ip"),
      NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF
          | NGX_CONF_TAKE1,
      ngx_http_access_rule,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("deny_x_request_client_real_ip"),
      NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF
          | NGX_CONF_TAKE1,
      ngx_http_access_rule,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    ngx_null_command
};


static ngx_http_module_t ngx_http_access2_module_ctx = {
    NULL,                 /* preconfiguration */
    ngx_http_access_init, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    ngx_http_access_create_loc_conf, /* create location configuration */
    ngx_http_access_merge_loc_conf   /* merge location configuration */
};


ngx_module_t ngx_http_access2_module = {
    NGX_MODULE_V1,
    &ngx_http_access2_module_ctx, /* module context */
    ngx_http_access2_commands,    /* module directives */
    NGX_HTTP_MODULE,              /* module type */
    NULL,                         /* init master */
    NULL,                         /* init module */
    NULL,                         /* init process */
    NULL,                         /* init thread */
    NULL,                         /* exit thread */
    NULL,                         /* exit process */
    NULL,                         /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_str_t client_real_ip_header_key = ngx_string("x-request-client-real-ip");

static ngx_int_t
ngx_http_access_handler(ngx_http_request_t *r)
{
    ngx_http_access_loc_conf_t *alcf;
    ngx_cidr_t cidr;
    ngx_int_t rc;
    ngx_list_part_t *part;
    ngx_table_elt_t *v;
    ngx_uint_t i;

    alcf = ngx_http_get_module_loc_conf(r, ngx_http_access2_module);

    ngx_memzero(&cidr, sizeof(ngx_cidr_t));


    part = &r->headers_in.headers.part;
    v    = part->elts;

    for (i = 0; /* void */; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }
            part = part->next;
            v    = part->elts;
            i    = 0;
        }

        if (ngx_strcasecmp(client_real_ip_header_key.data, v[i].key.data) == 0) {
            rc = ngx_ptocidr(&v[i].value, &cidr);

            if (rc == NGX_ERROR) {
                // log error only
                ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "invalid addr \"%V\"", &v[i].value);
            } else {
                switch (cidr.family) {
                    case AF_INET:
                        if (alcf->rules) {
                            return ngx_http_access_inet(r, alcf, cidr.u.in.addr);
                        }
                        break;
                }
            }

            break;
        }
    }

    return NGX_DECLINED;
}


static ngx_int_t
ngx_http_access_inet(ngx_http_request_t *r, ngx_http_access_loc_conf_t *alcf,
                     in_addr_t addr)
{
    ngx_uint_t i;
    ngx_http_access_rule_t *rule;

    rule = alcf->rules->elts;
    for (i = 0; i < alcf->rules->nelts; i++) {
        ngx_log_debug3(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "access: %08XD %08XD %08XD",
                       addr, rule[i].mask, rule[i].addr);

        if ((addr & rule[i].mask) == rule[i].addr) {
            return ngx_http_access_found(r, rule[i].deny);
        }
    }

    return NGX_DECLINED;
}

static ngx_int_t
ngx_http_access_found(ngx_http_request_t *r, ngx_uint_t deny)
{
    ngx_http_core_loc_conf_t *clcf;

    if (deny) {
        clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

        if (clcf->satisfy == NGX_HTTP_SATISFY_ALL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                          "access forbidden by rule");
        }

        return NGX_HTTP_FORBIDDEN;
    }

    return NGX_OK;
}


static char *
ngx_http_access_rule(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_access_loc_conf_t *alcf = conf;

    ngx_int_t rc;
    ngx_uint_t all;
    ngx_str_t *value;
    ngx_cidr_t cidr;
    ngx_http_access_rule_t *rule;

    all = 0;
    ngx_memzero(&cidr, sizeof(ngx_cidr_t));

    value = cf->args->elts;

    if (value[1].len == 3 && ngx_strcmp(value[1].data, "all") == 0) {
        all = 1;
    } else {
        rc = ngx_ptocidr(&value[1], &cidr);

        if (rc == NGX_ERROR) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid parameter \"%V\"", &value[1]);
            return NGX_CONF_ERROR;
        }

        if (rc == NGX_DONE) {
            ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                               "low address bits of %V are meaningless", &value[1]);
        }
    }

    if (cidr.family == AF_INET || all) {

        if (alcf->rules == NULL) {
            alcf->rules = ngx_array_create(cf->pool, 4,
                                           sizeof(ngx_http_access_rule_t));
            if (alcf->rules == NULL) {
                return NGX_CONF_ERROR;
            }
        }

        rule = ngx_array_push(alcf->rules);
        if (rule == NULL) {
            return NGX_CONF_ERROR;
        }

        rule->mask = cidr.u.in.mask;
        rule->addr = cidr.u.in.addr;
        rule->deny = (value[0].data[0] == 'd') ? 1 : 0;
    }
    return NGX_CONF_OK;
}


static void *
ngx_http_access_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_access_loc_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_access_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_http_access_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_access_loc_conf_t *prev = parent;
    ngx_http_access_loc_conf_t *conf = child;

    if (conf->rules == NULL) {
        conf->rules = prev->rules;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_access_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_ACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_access_handler;

    return NGX_OK;
}
