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
    apt-get install -yq git nginx-full  \
    php5.6-cli php5.6-fpm php5.6-common php5.6-mysql php5.6-bcmath \
    php5.6-gd php-memcached php5.6-redis \
    php5.6-curl php-pear php5.6-bz2 php5.6-imagick \
    php5.6-json php5.6-mbstring php5.6-soap php5.6-zip php5.6-xml \
    curl wget net-tools iputils-ping vim openssl strace \
    php5.6-dev libmcrypt-dev cron nodejs beanstalkd supervisor composer && \
    apt-get upgrade -y && \
    bash /tmp/setup_10.x && \
    apt-get install -y nodejs && \
    npm install -g n pm2 nuxt webpack cnpm && \ 
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/php-fpm.pid/g" /etc/php/5.6/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/php-fpm.log/g" /etc/php/5.6/fpm/php-fpm.conf && \
    pecl install mcrypt-snapshot && \
    echo "extension=mcrypt.so" > /etc/php/5.6/mods-available/20-mcrypt.ini && \
    ln -s /etc/php/5.6/mods-available/20-mcrypt.ini /etc/php/5.6/cli/conf.d/ && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && npm cache clean --force && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/5.6/cli/php.ini && \
    rm -rf /etc/php/5.6/fpm/php.ini && \
    groupadd admin && \
    useradd admin -g 1000
COPY nginx.conf /etc/nginx/nginx.conf
COPY php.ini /etc/php/5.6/cli/php.ini
COPY php.ini /etc/php/5.6/fpm/php.ini
EXPOSE 80
CMD tail /dev/null
