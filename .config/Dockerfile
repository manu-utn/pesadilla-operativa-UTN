FROM ubuntu:xenial
LABEL maintainer "neverkas"

USER root
ARG DEBIAN_FRONTEND=noninteractive

# variable de entorno que usaremos para cambiar el DIR_BASE del Makefile de los modulos
ENV DOCKER_RUNNING=true

RUN apt update && apt install -y \
    sudo git \
    make gcc gdb libcunit1 g++ libcunit1-dev \
    valgrind htop \
    vim nano \
    strace ssh make curl wget \
    libncurses5 tig autotools-dev libfuse-dev libreadline6-dev \
    build-essential
# libreadline6 libevent1-dev

RUN apt install -y \
    pkg-config autoconf automake \
    python3-docutils libseccomp-dev libjansson-dev libyaml-dev libxml2-dev

# necesario, porque en el release Xenial no se puede instalar mediante `apt install universal-ctags`
WORKDIR /tmp
RUN git clone https://github.com/universal-ctags/ctags.git && cd ctags && \
    ./autogen.sh && ./configure && make && sudo make install

WORKDIR /usr/src/cspec
RUN git clone http://github.com/mumuki/cspec .
RUN make clean all install

WORKDIR /usr/src/so-commons
RUN git clone https://github.com/sisoputnfrba/so-commons-library .
RUN make clean all test install

RUN useradd utnso
RUN adduser utnso sudo
RUN echo "utnso ALL=(ALL:ALL) ALL" >> /etc/sudoers
USER utnso

WORKDIR /home/utnso/tp
COPY . .

ENTRYPOINT ["/bin/bash"]
