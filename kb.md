# KB

## ngx_array_t

```C

ngx_array_t  *arr;

/* create an array of strings with preallocated memory for 2 elements */
arr = ngx_array_create(cf->pool, 2, sizeof(ngx_str_t));

ngx_str_t *s1 = ngx_array_push(arr);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p\n", arr->size, arr->nelts, arr);

ngx_str_t *s2 = ngx_array_push(arr);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p\n", arr->size, arr->nelts, arr);

ngx_str_t *s3 = ngx_array_push(arr);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p\n", arr->size, arr->nelts, arr);

ngx_str_t *sn = ngx_array_push_n(arr, 2);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p\n", arr->size, arr->nelts, arr);

```

```shell
nginx: [error] size: 16, len: 1, addr: 000000013682C090

nginx: [error] size: 16, len: 2, addr: 000000013682C090

nginx: [error] size: 16, len: 3, addr: 000000013682C090

nginx: [error] size: 16, len: 5, addr: 000000013682C090
```

## list header

```C
    ngx_list_part_t  *part;

    ngx_table_elt_t *header;

    size_t i;

    part = &r->headers_in.headers.part;
    header = part->elts;

    for (i = 0; /* void */; i++) {
        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            header = part->elts;
            i = 0;
        }
        ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "%d, %d", i, part->nelts);
        ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "%V: %V", &header[i].key, &header[i].value);
    }

```
