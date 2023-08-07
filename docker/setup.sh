#!/bin/bash
set -e

# sudo docker build -t docker-mysql .
# mysqld --initialize --user=root --basedir=/usr --datadir=/var/lib/mysql

sudo docker run -it --rm \
    --name test \
    -p 3306:3306 \
    -p 6379:6379 \
    -v /var/lib/mysql:/var/lib/mysql \
    -v /home/lux/dump.rdb:/dump.rdb \
    -v /home/lux/githubWorkplace/Lute/docker/setup.sh:/setup.sh \
    -v /home/lux/githubWorkplace/Lute:/Lute \
    lute



