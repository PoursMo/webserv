FROM ubuntu:jammy
RUN apt update
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get install -y bash
RUN apt-get install -y g++
RUN apt-get install -y telnet
RUN apt-get install -y libc-dev
RUN apt-get install -y libbsd-dev
RUN apt-get install -y zsh
RUN apt-get install -y nginx
RUN apt-get install -y valgrind
RUN apt-get install -y curl
RUN apt-get install -y netcat
RUN apt-get install -y git
RUN apt-get install -y make
RUN apt-get install -y jq
RUN apt-get install -y php-cgi
RUN apt-get install -y siege
RUN apt-get install -y python3.10
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
RUN unset DEBIAN_FRONTEND
RUN mkdir -p /app/logs/nginx
WORKDIR /app

ENTRYPOINT [ "/bin/zsh" ]
