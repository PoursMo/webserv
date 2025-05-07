FROM ubuntu:noble
RUN apt update
RUN apt-get install bash
RUN apt-get install g++ -y
RUN apt-get install telnet -y
RUN apt-get install netcat-traditional -y
RUN apt-get install libc-dev -y
RUN apt-get install libbsd-dev -y
RUN apt-get install zsh -y
RUN apt-get install nginx -y
WORKDIR /app

ENTRYPOINT [ "/bin/bash"]
