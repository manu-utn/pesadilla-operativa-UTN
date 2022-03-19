FROM ubuntu:latest
LABEL maintainer "neverkas"
USER root
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y \
    sudo git \
    make gcc gdb libcunit1 g++ libcunit1-dev \
    valgrind htop \
    vim nano \
    strace ssh make curl wget \
    libncurses5 tig autotools-dev libfuse-dev libreadline6-dev \
    build-essential
# libreadline6 libevent1-dev

WORKDIR /usr/src/cspec
RUN git clone http://github.com/mumuki/cspec .
RUN make clean all install

WORKDIR /usr/src/so-commons
RUN git clone https://github.com/sisoputnfrba/so-commons-library .
RUN make clean all test install

WORKDIR /home/utnso
COPY . ./project

WORKDIR /home/utnso/project

RUN useradd utnso
RUN adduser utnso sudo
RUN echo "utnso ALL=(ALL:ALL) ALL" >> /etc/sudoers
USER utnso

ENTRYPOINT ["/bin/bash"]
