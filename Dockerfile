# see: https://github.com/fideloper/docker-nginx-php/
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

ENV LANG       en_US.UTF-8
ENV LC_ALL     en_US.UTF-8

RUN sed -i "s/archive\.ubuntu\.com/mirrors.aliyun.com/g" /etc/apt/sources.list && \
    echo "Asia/Shanghai" > /etc/timezone && \
    apt-get update && \
    apt-get install -yq tzdata && \
    dpkg-reconfigure -f noninteractive tzdata && \
    apt-get install -yq locales && \
    locale-gen en_US.UTF-8 && \
    apt-get install -yq nginx \
    php-cli php-fpm php-common php-mysql php-bcmath \
    php-gd php-pdo php-memcached php-redis \
    php-curl php-pear php-bz2 php-imagick \
    php-json php-mbstring php-soap php-zip php-xml \
    curl wget net-tools iputils-ping vim openssl strace \
    php-dev libmcrypt-dev && \
    sed -i -e "s/;daemonize\s*=\s*yes/daemonize = no/g" /etc/php/7.2/fpm/php-fpm.conf && \
    sed -i "s/;cgi.fix_pathinfo\s*=\s*1/cgi.fix_pathinfo=0/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/;error_log\s*=\s*php_errors.log/error_log=\/data\/log\/php_errors.log/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/;error_log\s*=\s*php_errors.log/error_log=\/data\/log\/php_errors.log/g" /etc/php/7.2/cli/php.ini && \
    sed -i "s/;date.timezone\s*=/date.timezone=PRC/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/;date.timezone\s*=/date.timezone=PRC/g" /etc/php/7.2/cli/php.ini && \
    sed -i "s/post_max_size\s*=\s*8M/post_max_size=20M/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/post_max_size\s*=\s*8M/post_max_size=20M/g" /etc/php/7.2/cli/php.ini && \
    sed -i "s/;upload_tmp_dir\s*=/upload_tmp_dir=/data/log/tmp/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/;upload_tmp_dir\s*=/upload_tmp_dir=/data/log/tmp/g" /etc/php/7.2/cli/php.ini && \
    sed -i "s/upload_max_filesize\s*=\s*2M/upload_max_filesize=20M/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/upload_max_filesize\s*=\s*2M/upload_max_filesize=20M/g" /etc/php/7.2/cli/php.ini && \
    sed -i "s/;session.save_path\s*=\s*\".*\"/session.save_path=\"/data/log/tmp\"/g" /etc/php/7.2/fpm/php.ini && \
    sed -i "s/;session.save_path\s*=\s*\".*\"/session.save_path=\"/data/log/tmp\"/g" /etc/php/7.2/cli/php.ini && \
    pecl install mcrypt-snapshot && \
    echo "extension=mcrypt.so" > /etc/php/7.2/mods-available/20-mcrypt.ini && \
    ln -s /etc/php/7.2/mods-available/20-mcrypt.ini /etc/php/7.2/cli/conf.d/ && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
    rm -rf /etc/nginx/nginx.conf
COPY nginx.conf /etc/nginx/nginx.conf
EXPOSE 80
CMD tail /dev/null
