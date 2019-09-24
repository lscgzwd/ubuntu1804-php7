# see: https://github.com/lscgzwd/ubuntu1804-php7
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

ENV LANG       en_US.UTF-8
ENV LC_ALL     en_US.UTF-8
ENV TZ         Asia/Shanghai
COPY setup_10.x /tmp/setup_10.x
COPY wkhtmltox.deb /tmp/wkhtmltox.deb
COPY lib /tmp/lib
RUN groupadd -g 1000 pyds && useradd -s /bin/bash -g pyds pyds
RUN echo "root:piyuedashi2018" | chpasswd
RUN echo "pyds:piyuedashi2018" | chpasswd
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
        apt-get update && \
        apt-get install -yq tzdata software-properties-common && \
        dpkg-reconfigure -f noninteractive tzdata && \
        apt-get install -yq locales sudo && \
        locale-gen en_US.UTF-8
RUN echo "%pyds ALL=(ALL) ALL" >> /etc/sudoers
RUN apt-get install -yq curl wget net-tools iputils-ping vim openssl strace \
    cron beanstalkd supervisor openssh-server librsvg2* git traceroute \
    bash-completion samba openjdk-8-jdk xfonts-75dpi xfonts-base xfonts-utils \
    xfonts-encodings zip unzip gconf-service libasound2 libatk1.0-0 libatk-bridge2.0-0 \
    libc6 libcairo2 libcups2 libdbus-1-3 libexpat1 libfontconfig1 libgcc1 libgconf-2-4 \
    libgdk-pixbuf2.0-0 libglib2.0-0 libgtk-3-0 libnspr4 libpango-1.0-0 \
    libpangocairo-1.0-0 libstdc++6 libx11-6 libx11-xcb1 libxcb1 libxcomposite1 \
    libxcursor1 libxdamage1 libxext6 libxfixes3 libxi6 libxrandr2 libxrender1 \
    libxss1 libxtst6 ca-certificates fonts-liberation libappindicator1 libnss3 \
    lsb-release xdg-utils
RUN (echo piyuedashi2018;echo piyuedashi2018) | smbpasswd -a -s pyds
# inkscape
RUN add-apt-repository ppa:inkscape.dev/stable && apt-get update && apt-get install -yq inkscape
# nginx php
RUN add-apt-repository -y ppa:ondrej/php && \
    apt-get install -yq nginx-full php7.0-cli php7.0-fpm php7.0-common php7.0-mysql php7.0-bcmath \
    php7.0-gd php7.0-redis \
    php7.0-curl php7.0-bz2 php7.0-imagick php7.0-mcrypt \
    php7.0-json php7.0-mbstring php7.0-soap php7.0-zip php7.0-xml \
    php7.0-dev libmcrypt-dev composer && \
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/php-fpm.pid/g" /etc/php/7.0/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/php-fpm.log/g" /etc/php/7.0/fpm/php-fpm.conf && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    sed -i "s/rights=\"none\"\s*pattern=\"PDF\"/rights=\"read\|write\" pattern=\"PDF\"/g" /etc/ImageMagick-6/policy.xml && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/7.0/cli/php.ini && \
    rm -rf /etc/php/7.0/fpm/php.ini
# image_tool yimage
RUN apt-get install -yq libmagick++-6.q16-dev libmagick++-dev libmagickcore-6.q16-dev \
    libmagickcore-6.q16hdri-dev libmagickcore-dev libmagickwand-6.q16-dev \
    libmagickwand-6.q16hdri-dev libmagickwand-dev imagemagick-6.q16hdri \
    imagemagick-common imagemagick libmagick++-dev libmagickwand-dev \
    libboost-all-dev swig libjsoncpp-dev pagetools && \
    ln -sf /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick++-config /usr/bin/Magick++-config && \
    ln -sf /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick-config /usr/bin/Magick-config && \
    cd /tmp/lib/image-tool && make && make install && \
    cd /tmp/lib/yimage && make && make install
# wkhtmltox.deb export pdf
RUN dpkg -i /tmp/wkhtmltox.deb
# nodejs & npm
RUN bash /tmp/setup_10.x && \
    apt-get install -y nodejs && \
    npm install -g n pm2 nuxt webpack cnpm && \
    cnpm install -g puppeteer urlencode
# clean
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && npm cache clean --force
COPY nginx.conf /etc/nginx/nginx.conf
COPY php.ini /etc/php/7.0/cli/php.ini
COPY php.ini /etc/php/7.0/fpm/php.ini
CMD tail /dev/null

