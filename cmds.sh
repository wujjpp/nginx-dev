./auto/configure \
  --add-module=/root/nginx-dev/http-example-handler-module \
  --add-module=/root/nginx-dev/http-example-access-module \
  --add-module=/root/nginx-dev/http-example-filter-module \
  --add-module=/root/nginx-dev/http-example-upstream-module

./auto/configure \
  --with-stream \
  --add-dynamic-module=/root/nginx-dev/http-example-handler-module \
  --add-dynamic-module=/root/nginx-dev/http-example-access-module \
  --add-dynamic-module=/root/nginx-dev/http-example-filter-module \
  --add-dynamic-module=/root/nginx-dev/http-example-upstream-module
  