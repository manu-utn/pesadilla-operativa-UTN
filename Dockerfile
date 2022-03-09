FROM alpine:latest
LABEL maintainer "neverkas"

RUN apk update && apk add git make cgdb gcc musl-dev neovim screen valgrind
#RUN git clone http://github.com/syl20bnr/spacemacs ~/.emacs.d

WORKDIR /home/cspec
RUN git clone http://github.com/mumuki/cspec .
RUN make clean all install

WORKDIR /home/so-commons
RUN git clone https://github.com/sisoputnfrba/so-commons-library .
# como so-commons sólo contempla ubuntu, removemos el sudo
RUN sed -i 's/sudo //g' src/makefile
RUN make clean all test install

# si falla alguna dependencia del cgdb, descomentar esta otra linea
# apk add autoconf automake libtool flex bison gcc g++ ncurses ncurses-dev texinfo readline readline-dev

WORKDIR /home
COPY . ./data

WORKDIR /home/data

ENTRYPOINT ["/bin/sh", "-c"]
CMD ["make --no-print-directory watch"]
