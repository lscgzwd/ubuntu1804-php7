# see: https://github.com/fideloper/docker-nginx-php/
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

ENV LANG       en_US.UTF-8
ENV LC_ALL     en_US.UTF-8
ENV TZ=Asia/Shanghai
COPY setup_10.x /tmp/setup_10.x
# RUN sed -i "s/archive\.ubuntu\.com/mirrors.aliyun.com/g" /etc/apt/sources.list && \
RUN    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt-get update && \
    apt-get install -yq tzdata software-properties-common && \
    dpkg-reconfigure -f noninteractive tzdata && \
    add-apt-repository -y ppa:ondrej/php && \
    apt-get install -yq locales && \
    locale-gen en_US.UTF-8 && \
    apt-get install -yq git lua-nginx-redis lua-cjson-dev nginx-extras lua-nginx-redis-connector lua-nginx-string lua-nginx-cookie lua-nginx-kafka  \
    php7.2-cli php7.2-fpm php7.2-common php7.2-mysql php7.2-bcmath \
    php7.2-gd php-memcached php7.2-redis \
    php7.2-curl php-pear php7.2-bz2 php7.2-imagick \
    php7.2-json php7.2-mbstring php7.2-soap php7.2-zip php7.2-xml \
    curl wget net-tools iputils-ping vim openssl strace \
    php7.2-dev libmcrypt-dev cron nodejs beanstalkd supervisor composer && \
    apt-get upgrade -y && \
    bash /tmp/setup_10.x && \
    apt-get install -y nodejs && \
    npm install -g n pm2 nuxt webpack cnpm && \ 
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/tmp\/php-fpm.pid/g" /etc/php/7.2/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/tmp\/php-fpm.log/g" /etc/php/7.2/fpm/php-fpm.conf && \
    pecl install mcrypt-snapshot && \
    echo "extension=mcrypt.so" > /etc/php/7.2/mods-available/20-mcrypt.ini && \
    ln -s /etc/php/7.2/mods-available/20-mcrypt.ini /etc/php/7.2/cli/conf.d/ && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && npm cache clean --force && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/7.2/cli/php.ini && \
    rm -rf /etc/php/7.2/fpm/php.ini && \
    groupadd admin && \
    useradd admin -g 1000
COPY nginx.conf /etc/nginx/nginx.conf
COPY php.ini /etc/php/7.2/cli/php.ini
COPY php.ini /etc/php/7.2/fpm/php.ini
EXPOSE 80
CMD tail /dev/null
