# KB

## Containers

### ngx_array_t

```C

ngx_array_t  *arr;

/* create an array of strings with preallocated memory for 2 elements */
arr = ngx_array_create(cf->pool, 2, sizeof(ngx_str_t));

ngx_str_t *s1 = ngx_array_push(arr);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p", arr->size, arr->nelts, arr);

ngx_str_t *s2 = ngx_array_push(arr);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p", arr->size, arr->nelts, arr);

ngx_str_t *s3 = ngx_array_push(arr);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p", arr->size, arr->nelts, arr);

ngx_str_t *sn = ngx_array_push_n(arr, 2);
ngx_log_error(NGX_LOG_ERR, cf->log, 0, "size: %d, len: %d, addr: %p", arr->size, arr->nelts, arr);

```

size = 16, 是sizeof(ngx_str_t)的值

```shell
nginx: [error] size: 16, len: 1, addr: 000000013682C090
nginx: [error] size: 16, len: 2, addr: 000000013682C090
nginx: [error] size: 16, len: 3, addr: 000000013682C090
nginx: [error] size: 16, len: 5, addr: 000000013682C090
```

### ngx_list_t

```c
ngx_str_t        *s;
ngx_uint_t        i;
ngx_list_t       *list;
ngx_list_part_t  *part;

list = ngx_list_create(r->pool, 2, sizeof(ngx_str_t));
if (list == NULL) { /* error */ }

/* add items to the list */
s = ngx_list_push(list);
if (s == NULL) { /* error */ }
ngx_str_set(s, "foo");

s = ngx_list_push(list);
if (s == NULL) { /* error */ }
ngx_str_set(s, "bar");

s = ngx_list_push(list);
if (s == NULL) { /* error */ }
ngx_str_set(s, "should increse list part");


/* iterate over the list */

part = &list->part;
s = part->elts;

for (i = 0; /* void */; i++) {
    ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "part addr: %p, part->nelts: %d, part->elts: %p, part->next: %p", part, part->nelts, part->elts, part->next);

    if (i >= part->nelts) {
        if (part->next == NULL) {
            break;
        }

        part = part->next;
        s = part->elts;
        i = 0;
    }

    ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "s[%d]: %V", i, &s[i]);
}
```

```shell
2023/03/24 17:17:20 [error] 12537#79345: *1 part addr: 000000013000F458, part->nelts: 2, part->elts: 000000013000F490, part->next: 000000013000F4B0
2023/03/24 17:17:20 [error] 12537#79345: *1 s[0]: foo
2023/03/24 17:17:20 [error] 12537#79345: *1 part addr: 000000013000F458, part->nelts: 2, part->elts: 000000013000F490, part->next: 000000013000F4B0
2023/03/24 17:17:20 [error] 12537#79345: *1 s[1]: bar
2023/03/24 17:17:20 [error] 12537#79345: *1 part addr: 000000013000F458, part->nelts: 2, part->elts: 000000013000F490, part->next: 000000013000F4B0
2023/03/24 17:17:20 [error] 12537#79345: *1 s[0]: should increse list part
2023/03/24 17:17:20 [error] 12537#79345: *1 part addr: 000000013000F4B0, part->nelts: 1, part->elts: 000000013000F4D0, part->next: 0000000000000000

```

### ngx_queue_t

```c
typedef struct {
    ngx_str_t    value;
    ngx_queue_t  queue;
} ngx_foo_t;

ngx_foo_t    *f;
ngx_queue_t   values, *q;

ngx_queue_init(&values);

f = ngx_palloc(r->pool, sizeof(ngx_foo_t));
ngx_str_set(&f->value, "foo");
ngx_queue_insert_tail(&values, &f->queue);

f = ngx_palloc(r->pool, sizeof(ngx_foo_t));
ngx_str_set(&f->value, "bar");
ngx_queue_insert_tail(&values, &f->queue);

for (q = ngx_queue_head(&values); q != ngx_queue_sentinel(&values); q = ngx_queue_next(q))
{
    f = ngx_queue_data(q, ngx_foo_t, queue);
    ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "&f->value: %V", &f->value);
}
```

```shell
2023/03/24 17:35:25 [error] 15235#96652: *1 &f->value: foo
2023/03/24 17:35:25 [error] 15235#96652: *1 &f->value: bar
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
