
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
