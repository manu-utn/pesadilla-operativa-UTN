FROM alpine:latest
LABEL maintainer "neverkas"

RUN apk update && apk add git make cgdb gcc musl-dev neovim screen valgrind
#RUN git clone http://github.com/syl20bnr/spacemacs ~/.emacs.d

WORKDIR /home/cspec
RUN git clone http://github.com/mumuki/cspec .
RUN make clean && make && make install

# si falla alguna dependencia del cgdb, descomentar esta otra linea
# apk add autoconf automake libtool flex bison gcc g++ ncurses ncurses-dev texinfo readline readline-dev

WORKDIR /home
COPY . ./data

WORKDIR /home/data

ENTRYPOINT ["/bin/sh", "-c"]
CMD ["make --no-print-directory watch"]
