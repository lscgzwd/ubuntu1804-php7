# see: https://github.com/fideloper/docker-nginx-php/
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

RUN sed -i "s/archive\.ubuntu\.com/mirrors.aliyun.com/g" /etc/apt/sources.list
RUN echo "Asia/Shanghai" > /etc/timezone && apt-get update && apt-get install -y tzdata \
    && echo "Asia/Shanghai" > /etc/timezone && dpkg-reconfigure -f noninteractive tzdata
# Ensure UTF-8
RUN apt-get clean && apt-get update && apt-get install -y locales
RUN locale-gen en_US.UTF-8
ENV LANG       en_US.UTF-8
ENV LC_ALL     en_US.UTF-8

RUN apt-get update && apt-get install -y nginx \
    php php-cli php-fpm php-common php-mysql php-bcmath \
    php-gd php-pdo php-memcached php-redis \
    php-curl php-pear php-bz2 php-imagick php-dba \
    php-json php-mbstring php-soap php-zip php-xml \
    curl wget net-tools iputils-ping vim openssl
RUN echo "daemon off;" >> /etc/nginx/nginx.conf
RUN sed -i -e "s/;daemonize\s*=\s*yes/daemonize = no/g" /etc/php/7.2/fpm/php-fpm.conf
RUN sed -i "s/;cgi.fix_pathinfo\s*=\s*1/cgi.fix_pathinfo=0/g" /etc/php/7.2/fpm/php.ini
RUN sed -i "s/;error_log\s*=\s*php_errors.log/error_log=\/data\/log\/php_errors.log/g" /etc/php/7.2/fpm/php.ini
RUN sed -i "s/;error_log\s*=\s*php_errors.log/error_log=\/data\/log\/php_errors.log/g" /etc/php/7.2/cli/php.ini
RUN sed -i "s/;date.timezone\s*=/date.timezone=PRC/g" /etc/php/7.2/fpm/php.ini
RUN sed -i "s/;date.timezone\s*=/date.timezone=PRC/g" /etc/php/7.2/cli/php.ini
RUN pecl install mcrypt

RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

EXPOSE 80
CMD tail /dev/null
