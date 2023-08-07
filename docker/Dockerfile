FROM ubuntu:20.04

# Set default shell to /bin/bash
SHELL ["/bin/bash", "-euo", "pipefail", "-c"]


# Prepare to install specific g++ versions
RUN apt-get update -qq && apt-get install -y --no-install-recommends software-properties-common

# ENV DEBIAN_FRONTEND noninteractive

# Install essential packages.
RUN apt-get update && apt-get install -y \
    build-essential \
    pkg-config \
    cmake \
    g++ \
    git \
    curl \
    vim \
    wget \
    net-tools \
    mysql-server \
    libmysqlclient-dev \
    redis-server \
    && apt-get clean && rm -rf /var/lib/apt/lists/*


# Allow OpenSSH to talk to containers without asking for confirmation
RUN mkdir -p /var/run/sshd
RUN cat /etc/ssh/ssh_config | grep -v StrictHostKeyChecking > /etc/ssh/ssh_config.new && \
    echo "    StrictHostKeyChecking no" >> /etc/ssh/ssh_config.new && \
    mv /etc/ssh/ssh_config.new /etc/ssh/ssh_config

EXPOSE 3306 6379

# RUN mysqld --initialize --user=root --basedir=/usr --datadir=/var/lib/mysql
# RUN chmod -R 777 /var/lib/mysql

# COPY /usr/local/etc/redis.conf /redis.conf

RUN service mysql stop
RUN usermod -d /var/lib/mysql/ mysql
RUN mysqld --initialize --user=root --basedir=/usr --datadir=/var/lib/mysql

# Install hiredis
RUN git clone https://github.com/redis/hiredis.git --depth 1
RUN cd hiredis
RUN cmake -B build
RUN cmake --build build
RUN make install

RUN service mysql start 
RUN service redis-server start


