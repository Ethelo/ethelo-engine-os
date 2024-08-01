FROM ubuntu:jammy

# Installs BONMIN
RUN apt-get update
RUN apt-get -y install --no-install-recommends git subversion gcc g++ make wget gfortran patch pkg-config file
RUN apt-get -y install --no-install-recommends libgfortran-10-dev libblas-dev liblapack-dev libmetis-dev libnauty2-dev
RUN apt-get -y install --no-install-recommends ca-certificates
RUN apt-get clean
RUN apt-get autoremove

RUN git clone https://github.com/coin-or/coinbrew /var/coin
WORKDIR /var/coin

RUN ./coinbrew fetch BONMIN@master --no-prompt
RUN ./coinbrew build BONMIN -t main --no-prompt --prefix=/usr

# Link to account for change of namespace
RUN ln -s /usr/include/coin-or /usr/include/coin

# Install dependencies
RUN apt-get update && apt-get install -y \
       wget \
       git \
       locales \
       cmake \
       build-essential \
       liblapack-dev \
       liblapack3 \
       libeigen3-dev \
       libopenblas-dev libarpack2-dev libsuperlu-dev \
       libarmadillo10 \
       libarmadillo-dev \
       gfortran \
       antlr3 \
       gdb \
       valgrind \
       
    && sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && echo 'LANG="en_US.UTF-8"'>/etc/default/locale \
    && dpkg-reconfigure --frontend=noninteractive locales \
    && /usr/sbin/update-locale LANG=en_US.UTF-8

# Install Erlang OTP 26 from Erlang Solutions
RUN wget https://binaries2.erlang-solutions.com/ubuntu/pool/contrib/e/esl-erlang/esl-erlang_26.2.3-1~ubuntu~jammy_amd64.deb \
    && apt-get install -y ./esl-erlang_26.2.3-1~ubuntu~jammy_amd64.deb \
    && rm esl-erlang_26.2.3-1~ubuntu~jammy_amd64.deb

# Update Erlang paths
RUN echo "/usr/lib/erlang/lib/erl_interface-*/lib" > /etc/ld.so.conf.d/erlang.conf \
    && ldconfig

# Set Erlang environment variables
ENV ERLANG_HOME=/usr/lib/erlang
ENV PATH=$PATH:$ERLANG_HOME/bin

# Clean up
RUN apt-get clean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENV LANG en_US.UTF-8
ENV MAKEOVERRIDES -j

COPY ./3rdparty /tmp/3p
WORKDIR /tmp/3p/
RUN ./ethelo.sh && rm -rf *


VOLUME /app
WORKDIR /app
