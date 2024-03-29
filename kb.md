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

### ngx_rbtree_t

#### base

```c
ngx_rbtree_node_t *
ngx_value_rbtree_lookup(ngx_rbtree_t *rbtree, uint32_t hash)
{
    ngx_int_t           rc;
    ngx_rbtree_node_t  *n;
    ngx_rbtree_node_t  *node, *sentinel;

    node = rbtree->root;
    sentinel = rbtree->sentinel;

    while (node != sentinel) {

        n = (ngx_rbtree_node_t *) node;

        if (hash != node->key) {
            node = (hash < node->key) ? node->left : node->right;
            continue;
        }
        return n;
    }

    return NULL;
}

static ngx_int_t
ngx_rbtree_test(ngx_http_request_t *r, ...)
{
    ngx_int_t           i;
    ngx_rbtree_t        tree;
    ngx_rbtree_node_t   sentinel;
    ngx_rbtree_node_t  *node;

    ngx_rbtree_init(&tree, &sentinel, ngx_rbtree_insert_value);

    for(i = 1; i <= 5; ++i) {
        node = ngx_palloc(r->pool, sizeof(ngx_rbtree_node_t));
        node->key = i;

        ngx_rbtree_insert(&tree, node);
    }

    node = ngx_rbtree_min(tree.root, &sentinel);
    ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "min node key: %d", node->key);

    node = ngx_value_rbtree_lookup(&tree, 2);

    if(node != NULL) {
        ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node key: %d", node->key);

        ngx_rbtree_delete(&tree, node);

        node = ngx_value_rbtree_lookup(&tree, 2);
        if(node != NULL) {
            ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node after delete, key: %d", node->key);
        } else {
            ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node after delete: NOT FOUND");
        }
    } else {
        ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node: NOT FOUND");
    }
}
```

```shell

2023/03/27 18:29:07 [error] 71140#892965: *3 min node key: 1
2023/03/27 18:29:07 [error] 71140#892965: *3 search node key: 2
2023/03/27 18:29:07 [error] 71140#892965: *3 search node after delete: NOT FOUND

```

#### ngx_str_rbtree

```c
ngx_int_t           i;
ngx_rbtree_t        tree;
ngx_rbtree_node_t   sentinel;

ngx_str_node_t     *node;
ngx_str_t          *node_name;

ngx_rbtree_init(&tree, &sentinel, ngx_str_rbtree_insert_value);

for(i = 1; i <= 5; ++i) {
    node = ngx_palloc(r->pool, sizeof(ngx_str_node_t));

    node_name = ngx_palloc(r->pool, sizeof(ngx_str_t));
    node_name->data = ngx_pnalloc(r->pool, 7);
    node_name->len = 6;

    ngx_sprintf(node_name->data, "node-%d", i);

    node->str = *node_name;
    node->node.key = i;

    ngx_rbtree_insert(&tree, &node->node);
}

node = (ngx_str_node_t *)ngx_rbtree_min(tree.root, &sentinel);
ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "min node: %V, key: %d", &(node->str), node->node.key);

ngx_str_t search_key = ngx_string("node-2");

node = (ngx_str_node_t *)ngx_str_rbtree_lookup(&tree, &search_key, 2);

if(node != NULL) {
    ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node: %V, key: %d", &node->str, node->node.key);

    ngx_rbtree_delete(&tree, &node->node);

    node = (ngx_str_node_t *)ngx_str_rbtree_lookup(&tree, &search_key, 2);
    if(node != NULL) {
        ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node after delete: %V, key: %d", &node->str, node->node.key);
    } else {
        ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node after delete: NOT FOUND");
    }
} else {
    ngx_log_error(NGX_LOG_ERR, r->pool->log, 0, "search node: NOT FOUND");
}
```

```shell

2023/03/27 17:48:34 [error] 68855#856319: *2 min node: node-1, key: 1
2023/03/27 17:48:34 [error] 68855#856319: *2 search node: node-2, key: 2
2023/03/27 17:48:34 [error] 68855#856319: *2 search node after delete: NOT FOUND

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
