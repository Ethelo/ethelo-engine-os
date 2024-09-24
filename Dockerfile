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
       unzip \
       inotify-tools \
       libelf1 libelf-dev \
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

# fix Mac M2 Chip errors ( https://elixirforum.com/t/cant-compile-elixir-phoenix-api-in-docker-build-step/56863/6 )
ENV ERL_FLAGS="+JPperf true"
# Clean up
RUN apt-get clean \
    && apt-get autoremove \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENV LANG en_US.UTF-8
ENV MAKEOVERRIDES -j

COPY ./3rdparty /tmp/3p
WORKDIR /tmp/3p/
RUN ./ethelo.sh && rm -rf *

# Copy the source code to /app
COPY . /app

ENV LD_LIBRARY_PATH=/app/build/lib:$LD_LIBRARY_PATH
# Build driver
WORKDIR /app/build
RUN cmake /app && make


# Download and Install Specific Version of Elixir

ARG ELIXIR_VERSION=v1.17.2
ARG OTP_VERSION=elixir-otp-26

RUN mkdir -p /app/elixir
WORKDIR /app/elixir
RUN wget -q https://github.com/elixir-lang/elixir/releases/download/${ELIXIR_VERSION}/${OTP_VERSION}.zip \
  && unzip ${OTP_VERSION}.zip \
  && rm -f ${OTP_VERSION}.zip \
  && ln -s /app/elixir/bin/elixirc /usr/local/bin/elixirc \
  && ln -s /app/elixir/bin/elixir /usr/local/bin/elixir \
  && ln -s /app/elixir/bin/mix /usr/local/bin/mix \
  && ln -s /app/elixir/bin/iex /usr/local/bin/iex

RUN echo yes | mix do local.hex, local.rebar, archive.install github hexpm/hex branch latest
RUN echo yes | mix do hex phoenix 1.7.14

# Move driver to Elixir
COPY ./elixir_engine /app/elixir_engine

# Ensure the target directory exists
RUN mkdir -p /app/elixir_engine/priv/bin

# Move the driver
RUN mv /app/build/bin/ethelo_driver /app/elixir_engine/priv/bin/ethelo_driver

WORKDIR /app/elixir_engine

# Set environment variables and build Elixir project
ENV MIX_ENV=prod
RUN mix deps.get
RUN mix compile

VOLUME /app
WORKDIR /app
