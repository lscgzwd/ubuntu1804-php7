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
    php7.0-cli php7.0-fpm php7.0-common php7.0-mysql php7.0-bcmath \
    php7.0-gd php-memcached php7.0-redis \
    php7.0-curl php-pear php7.0-bz2 php7.0-imagick php7.0-mcrypt \
    php7.0-json php7.0-mbstring php7.0-soap php7.0-zip php7.0-xml \
    curl wget net-tools iputils-ping vim openssl strace \
    php7.0-dev libmcrypt-dev cron nodejs beanstalkd supervisor composer && \
    apt-get upgrade -y && \
    bash /tmp/setup_10.x && \
    apt-get install -y nodejs && \
    npm install -g n pm2 nuxt webpack cnpm && \ 
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/php-fpm.pid/g" /etc/php/7.0/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/php-fpm.log/g" /etc/php/7.0/fpm/php-fpm.conf && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && npm cache clean --force && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/7.0/cli/php.ini && \
    rm -rf /etc/php/7.0/fpm/php.ini && \
    groupadd admin && \
    useradd admin -g 1000
COPY nginx.conf /etc/nginx/nginx.conf
COPY php.ini /etc/php/7.0/cli/php.ini
COPY php.ini /etc/php/7.0/fpm/php.ini
EXPOSE 80
CMD tail /dev/null
