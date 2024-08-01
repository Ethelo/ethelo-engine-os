FROM ubuntu:xenial


RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
       wget\
       git\
       locales\
       cmake\
       build-essential\
       liblapack-dev\
       liblapack3\
       libeigen3-dev\
       libarmadillo6\
       libarmadillo-dev\
       gfortran\
       antlr3\
       erlang-dev\
       gdb\
       valgrind\
    && sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && echo 'LANG="en_US.UTF-8"'>/etc/default/locale \
    && dpkg-reconfigure --frontend=noninteractive locales \
    && /usr/sbin/update-locale LANG=en_US.UTF-8 \
    && apt-get clean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* 

ENV LANG en_US.UTF-8
ENV MAKEOVERRIDES -j

COPY ./3rdparty /tmp/3p
WORKDIR /tmp/3p/
RUN ./ethelo.sh && rm -rf *


VOLUME /app
WORKDIR /app
