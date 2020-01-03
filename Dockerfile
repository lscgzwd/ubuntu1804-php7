# see: https://github.com/lscgzwd/ubuntu1804-php7
FROM ubuntu:18.04
MAINTAINER i@shuncheng.lu

ENV LANG       en_US.UTF-8
ENV LC_ALL     en_US.UTF-8
ENV TZ         Asia/Shanghai
VOLUME ["/tmp", "/var/log", "/var/cache"]
COPY pkg /tmp/pkg
COPY lib /tmp/lib
RUN groupadd -g 1000 pyds && useradd -s /bin/bash -g pyds pyds && echo "root:piyuedashi2018" | chpasswd && echo "pyds:piyuedashi2018" | chpasswd
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && \
    rm -rf /etc/apt/sources.list && cp /tmp/pkg/sources.list /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -yq tzdata software-properties-common && \
    dpkg-reconfigure -f noninteractive tzdata && \
    apt-get install -yq locales sudo \
    curl wget net-tools iputils-ping vim openssl strace \
    cron beanstalkd supervisor openssh-server librsvg2* git traceroute \
    bash-completion samba openjdk-8-jdk xfonts-75dpi xfonts-base xfonts-utils \
    xfonts-encodings zip unzip telnet && \
    apt-get install -yq gconf-service libasound2 libatk1.0-0 libatk-bridge2.0-0 \
    libc6 libcairo2 libcups2 libdbus-1-3 libexpat1 libfontconfig1 libgcc1 libgconf-2-4 \
    libgdk-pixbuf2.0-0 libglib2.0-0 libgtk-3-0 libnspr4 libpango-1.0-0 \
    libpangocairo-1.0-0 libstdc++6 libx11-6 libx11-xcb1 libxcb1 libxcomposite1 \
    libxcursor1 libxdamage1 libxext6 libxfixes3 libxi6 libxrandr2 libxrender1 \
    libxss1 libxtst6 ca-certificates fonts-liberation libappindicator1 libnss3 \
    lsb-release xdg-utils && apt-get clean && \
    locale-gen en_US.UTF-8
RUN echo "%pyds ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers && (echo piyuedashi2018;echo piyuedashi2018) | smbpasswd -a -s pyds
# opencv4.1.2
RUN cd /tmp/pkg/ && make install
# golang 1.13.4
RUN cd /tmp && curl -Lo go.tar.gz https://gomirrors.org/dl/go/go1.13.4.linux-amd64.tar.gz && \
    tar xf go.tar.gz && mv go /usr/local/ && \
    echo "export GOROOT=/usr/local/go              # 安装目录。\nexport GOPATH=/data/go     # 工作环境\nexport GOBIN=\$GOPATH/bin           # 可执行文件存放\nexport PATH=\$GOPATH:\$GOBIN:\$GOROOT/bin:\$PATH       # 添加PATH路径" >> /etc/bash.bashrc
# inkscape
RUN add-apt-repository ppa:inkscape.dev/stable && apt-get update && apt-get install -yq inkscape && apt-get clean
# ImageMagick
RUN apt-get install -qy libopenexr-dev libwmf-dev libxml2-dev libbz2-dev libdjvulibre-dev libexif-dev \
    libjpeg-dev libpng-dev  liblcms2-dev liblqr-1-0-dev libperl-dev libwebp-dev librsvg2-dev libgif-dev  && \
    cd /tmp && curl -Lo ImageMagick.tar.gz https://code.aliyun.com/lscgzwd/raw/raw/master/ImageMagick.tar.gz && \
    tar xf ImageMagick.tar.gz && cd ImageMagick-7.0.9-12 && \
    ./configure --prefix=/usr --sysconfdir=/etc --enable-hdri --with-modules --with-jemalloc=yes --with-perl && \
    make && make install && ln -snf /usr/bin/MagickCore-config /usr/bin/Magick-config
# nginx php
RUN add-apt-repository -y ppa:ondrej/php && apt-get update && \
    apt-get install -yq nginx-full php7.0-cli php7.0-fpm php7.0-common php7.0-mysql php7.0-bcmath \
    php7.0-gd php7.0-redis \
    php7.0-curl php7.0-bz2 php7.0-mcrypt \
    php7.0-json php7.0-mbstring php7.0-soap php7.0-zip php7.0-xml \
    php7.0-dev libmcrypt-dev composer && \
    pecl install Imagick && \
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/php-fpm.pid/g" /etc/php/7.0/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/php-fpm.log/g" /etc/php/7.0/fpm/php-fpm.conf && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    sed -i "s/rights=\"none\"\s*pattern=\"PDF\"/rights=\"read\|write\" pattern=\"PDF\"/g" /etc/ImageMagick-6/policy.xml && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/7.0/cli/php.ini && \
    rm -rf /etc/php/7.0/fpm/php.ini && \
    cp /tmp/pkg/php.ini /etc/php/7.0/cli/php.ini && \
    cp /tmp/pkg/php.ini /etc/php/7.0/fpm/php.ini && \
    cp /tmp/pkg/nginx.conf /etc/nginx/nginx.conf
# image_tool yimage
RUN apt-get install -yq libboost-all-dev swig libjsoncpp-dev pagetools && \
    cd /tmp/lib/image-tool && make clean && make && make install && \
    cd /tmp/lib/yimage && make clean && make && make install
# nodejs & npm
RUN bash /tmp/pkg/setup_10.x && \
    apt-get update && apt-get install -y nodejs && apt-get clean && \
    npm install -g n pm2 nuxt webpack cnpm --registry=https://registry.npm.taobao.org && \
    npm config set puppeteer_download_host=https://npm.taobao.org/mirrors && \
    npm install -g puppeteer urlencode --unsafe-perm=true --registry=https://registry.npm.taobao.org && npm cache clean --force
# clean
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
CMD tail /dev/null

