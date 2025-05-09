FROM ubuntu:jammy
RUN apt update
RUN apt-get install bash
RUN apt-get install g++ -y
RUN apt-get install telnet -y
RUN apt-get install libc-dev -y
RUN apt-get install libbsd-dev -y
RUN apt-get install zsh -y
RUN apt-get install nginx -y
RUN apt-get install valgrind -y
RUN apt-get install curl -y
RUN apt-get install netcat -y
RUN apt-get install git -y
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
WORKDIR /app

ENTRYPOINT [ "/bin/zsh" ]
