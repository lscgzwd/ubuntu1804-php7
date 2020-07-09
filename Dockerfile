# see: https://github.com/fideloper/docker-nginx-php/
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

VOLUME ["/tmp", "/var/cache"]
ENV LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANGUAGE=en_US.UTF-8 TZ=Asia/Shanghai
COPY * /tmp/
RUN sed -i "s/archive\.ubuntu\.com/mirrors.aliyun.com/g" /etc/apt/sources.list && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt-get update && \
    apt-get install -yq tzdata software-properties-common && \
    dpkg-reconfigure -f noninteractive tzdata && \
    apt-get install -yq locales && \
    locale-gen en_US.UTF-8 && \
    apt-get install -yq lua-nginx-redis lua-cjson-dev nginx-extras lua-nginx-redis-connector lua-nginx-string lua-nginx-cookie lua-nginx-kafka  \
    curl wget net-tools iputils-ping vim openssl && \
    apt-get upgrade -y && \
    rm -rf /etc/nginx/nginx.conf && \
    cp /tmp/nginx.conf /etc/nginx/nginx.conf && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    groupadd admin && \
    useradd admin -g 1000
EXPOSE 80
CMD tail /dev/null
