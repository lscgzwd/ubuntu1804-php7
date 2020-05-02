# see: https://github.com/fideloper/docker-nginx-php/
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

VOLUME ["/tmp", "/var/cache"]
ENV LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 LANGUAGE=en_US.UTF-8 TZ=Asia/Shanghai
COPY * /tmp/
# RUN sed -i "s/archive\.ubuntu\.com/mirrors.aliyun.com/g" /etc/apt/sources.list && \
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    apt-get update && \
    apt-get install -yq tzdata software-properties-common && \
    dpkg-reconfigure -f noninteractive tzdata && \
    apt-get install -yq locales && \
    locale-gen en_US.UTF-8 && \
    apt-get install -yq git nginx-full  \
    php-cli php-fpm php-common php-mysql php-bcmath \
    php-gd php-memcached php-redis \
    php-curl php-pear php-bz2 php-imagick \
    php-json php-mbstring php-soap php-zip php-xml \
    curl wget net-tools iputils-ping vim openssl strace \
    php-dev libmcrypt-dev cron nodejs && \
    apt-get upgrade -y && \
    bash /tmp/setup_12.x && \
    apt-get install -y nodejs && \
    npm install -g n pm2 && \ 
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/tmp\/php-fpm.pid/g" /etc/php/7.2/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/tmp\/php-fpm.log/g" /etc/php/7.2/fpm/php-fpm.conf && \
    pecl install mcrypt-snapshot && \
    echo "extension=mcrypt.so" > /etc/php/7.2/mods-available/20-mcrypt.ini && \
    ln -s /etc/php/7.2/mods-available/20-mcrypt.ini /etc/php/7.2/cli/conf.d/ && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/7.2/cli/php.ini && \
    rm -rf /etc/php/7.2/fpm/php.ini && \
    cp /tmp/nginx.conf /etc/nginx/nginx.conf && \
    cp /tmp/php.ini /etc/php/7.2/cli/php.ini && \
    cp /tmp/php.ini /etc/php/7.2/fpm/php.ini && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && npm cache clean --force && \
    groupadd admin && \
    useradd admin -g 1000
EXPOSE 80
CMD tail /dev/null
