./configure \
  --with-stream \
  --with-http_ssl_module \
  --with-http_realip_module \
  --with-stream_ssl_module \
  --with-stream_realip_module \
  --add-dynamic-module=/root/nginx-dev/ngx_devel_kit \
  --add-dynamic-module=/root/nginx-dev/njs/nginx \
  --add-dynamic-module=/root/nginx-dev/redis2-nginx-module \
  --add-dynamic-module=/root/nginx-dev/set-misc-nginx-module \
  --add-module=/root/nginx-dev/nginx-module-vts

./auto/configure \
  --add-module=/root/nginx-dev/http-example-handler-module \
  --add-module=/root/nginx-dev/http-example-access-module \
  --add-module=/root/nginx-dev/http-example-filter-module

./auto/configure \
  --add-dynamic-module=/root/nginx-dev/http-example-handler-module \
  --add-dynamic-module=/root/nginx-dev/http-example-access-module \
  --add-dynamic-module=/root/nginx-dev/http-example-filter-module
  