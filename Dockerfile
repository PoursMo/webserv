FROM alpine:3
RUN apk add --no-cache zsh
RUN apk add --no-cache bash
RUN apk add --no-cache valgrind
RUN apk add --no-cache curl
RUN apk add --no-cache git
RUN apk add --no-cache g++
RUN apk add --no-cache make
RUN apk add --no-cache gdb
RUN apk add --no-cache mandoc man-pages
RUN apk add --no-cache libc-dev
RUN apk add --no-cache libbsd-dev
RUN apk add --no-cache php84-cgi
RUN apk add --no-cache jq
RUN apk add --no-cache inetutils-telnet
RUN apk add --no-cache nginx
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
WORKDIR /app
ENTRYPOINT [ "/bin/zsh" ]
