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
    xfonts-encodings zip unzip gconf-service libasound2 libatk1.0-0 libatk-bridge2.0-0 \
    libc6 libcairo2 libcups2 libdbus-1-3 libexpat1 libfontconfig1 libgcc1 libgconf-2-4 \
    libgdk-pixbuf2.0-0 libglib2.0-0 libgtk-3-0 libnspr4 libpango-1.0-0 \
    libpangocairo-1.0-0 libstdc++6 libx11-6 libx11-xcb1 libxcb1 libxcomposite1 \
    libxcursor1 libxdamage1 libxext6 libxfixes3 libxi6 libxrandr2 libxrender1 \
    libxss1 libxtst6 ca-certificates fonts-liberation libappindicator1 libnss3 \
    lsb-release xdg-utils telnet && apt-get clean && \
    locale-gen en_US.UTF-8
RUN echo "%pyds ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers && (echo piyuedashi2018;echo piyuedashi2018) | smbpasswd -a -s pyds
# opencv4.1.2
RUN cd /tmp/pkg/ && make install
# golang 1.13.4
RUN cd /tmp && wget -O go.tar.gz https://studygolang.com/dl/golang/go1.13.4.linux-amd64.tar.gz && \
    tar xf go.tar.gz && mv go /usr/local/ && \
    echo "export GOROOT=/usr/local/go              # 安装目录。\nexport GOPATH=/data/go     # 工作环境\nexport GOBIN=\$GOPATH/bin           # 可执行文件存放\nexport PATH=\$GOPATH:\$GOBIN:\$GOROOT/bin:\$PATH       # 添加PATH路径" >> /etc/bash.bashrc
# inkscape
RUN add-apt-repository ppa:inkscape.dev/stable && apt-get update && apt-get install -yq inkscape && apt-get clean
# nginx php
RUN add-apt-repository -y ppa:ondrej/php && apt-get update && \
    apt-get install -yq nginx-full php5.6-cli php5.6-fpm php5.6-common php5.6-mysql php5.6-bcmath \
    php5.6-gd php5.6-redis \
    php5.6-curl php5.6-bz2 php5.6-imagick php5.6-mcrypt \
    php5.6-json php5.6-mbstring php5.6-soap php5.6-zip php5.6-xml \
    php5.6-dev libmcrypt-dev composer && \
    sed -i "s/pid\s*=\s*.*pid/pid=\/data\/log\/php-fpm.pid/g" /etc/php/5.6/fpm/php-fpm.conf && \
    sed -i "s/error_log\s*=\s*.*fpm.log/error_log=\/data\/log\/php-fpm.log/g" /etc/php/5.6/fpm/php-fpm.conf && \
    echo "fastcgi_split_path_info ^(.+\\.php)(/.+)\$; #增加这一句 \nfastcgi_param PATH_INFO \$fastcgi_path_info; #增加这一句 \nfastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;" >> /etc/nginx/fastcgi_params && \
    sed -i "s/rights=\"none\"\s*pattern=\"PDF\"/rights=\"read\|write\" pattern=\"PDF\"/g" /etc/ImageMagick-6/policy.xml && \
    rm -rf /etc/nginx/nginx.conf && \
    rm -rf /etc/php/5.6/cli/php.ini && \
    rm -rf /etc/php/5.6/fpm/php.ini && \
    cp /tmp/pkg/php.ini /etc/php/5.6/cli/php.ini && \
    cp /tmp/pkg/php.ini /etc/php/5.6/fpm/php.ini && \
    cp /tmp/pkg/nginx.conf /etc/nginx/nginx.conf
# image_tool yimage
RUN apt-get install -yq libmagick++-6.q16-dev libmagick++-dev libmagickcore-6.q16-dev \
    libmagickcore-6.q16hdri-dev libmagickcore-dev libmagickwand-6.q16-dev \
    libmagickwand-6.q16hdri-dev libmagickwand-dev imagemagick-6.q16hdri \
    imagemagick-common imagemagick libmagick++-dev libmagickwand-dev \
    libboost-all-dev swig libjsoncpp-dev pagetools && \
    ln -sf /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick++-config /usr/bin/Magick++-config && \
    ln -sf /usr/lib/x86_64-linux-gnu/ImageMagick-6.9.7/bin-q16/Magick-config /usr/bin/Magick-config && \
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

