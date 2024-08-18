FROM alpine

RUN apk update && apk add --no-cache libc6-compat libstdc++
RUN adduser --system server;
RUN mkdir -p /var/www/{test,coucou,hidden}

COPY index_all.html /var/www/index.html
COPY index_test.html /var/www/test/index.html
COPY index_coucou.html /var/www/coucou/index.html
COPY index_hidden.html /var/www/hidden/index.html
COPY test_config.yml /tmp/test_config.yml
RUN chown -R server /var/www

# USER server
ENTRYPOINT ["/home/server/webserv", "/tmp/test_config.yml"]
