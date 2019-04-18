# see: https://github.com/lscgzwd/ubuntu1804-php7
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

ENV LANG       en_US.UTF-8
ENV LC_ALL     en_US.UTF-8
ENV TZ         Asia/Shanghai
COPY setup_10.x /tmp/setup_10.x
RUN groupadd -g 1000 pyds && useradd -s /bin/bash -g pyds pyds
RUN echo "root:piyuedashi2018" | chpasswd
RUN echo "pyds:piyuedashi2018" | chpasswd
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
        apt-get update && \
        apt-get install -yq tzdata software-properties-common && \
        dpkg-reconfigure -f noninteractive tzdata && \
        apt-get install -yq locales && \
        locale-gen en_US.UTF-8
RUN apt-get install -yq curl wget net-tools iputils-ping vim openssl strace \
    cron beanstalkd supervisor composer openssh-server librsvg2* git traceroute \
    bash-completion samba
RUN echo -e "piyuedashi2018\npiyuedashi2018" | smbpasswd -a -s pyds
# nginx php
RUN add-apt-repository -y ppa:ondrej/php && \
    apt-get install -yq php7.0-cli php7.0-fpm php7.0-common php7.0-mysql php7.0-bcmath \
    php7.0-gd php-memcached php7.0-redis \
    php7.0-curl php-pear php7.0-bz2 php7.0-imagick php7.0-mcrypt \
    php7.0-json php7.0-mbstring php7.0-soap php7.0-zip php7.0-xml \
    php7.0-dev libmcrypt-dev && \
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/php-fpm.pid/g" /etc/php/7.0/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/php-fpm.log/g" /etc/php/7.0/fpm/php-fpm.conf && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/7.0/cli/php.ini && \
    rm -rf /etc/php/7.0/fpm/php.ini
# nodejs & npm
RUN bash /tmp/setup_10.x && \
    apt-get install -y nodejs && \
    npm install -g n pm2 nuxt webpack cnpm
# clean
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && npm cache clean --force
COPY nginx.conf /etc/nginx/nginx.conf
COPY php.ini /etc/php/7.0/cli/php.ini
COPY php.ini /etc/php/7.0/fpm/php.ini
CMD tail /dev/null
