#include <iostream>
#include <poll.h>
#include <vector>
#include <fcntl.h>

int main()
{
    std::vector<struct pollfd> pollFds;
    int fd = open("Makefile", O_RDONLY);
    pollFds.push_back((struct pollfd) {
        .fd = fd,
            .events = POLLIN,
            .revents = 0
    });
    fd = open("/dev/null", O_WRONLY);
    pollFds.push_back((struct pollfd) {
        .fd = fd,
            .events = POLLOUT,
            .revents = 0
    });
    poll(pollFds.data(), pollFds.size(), -1);
    std::vector<struct pollfd> fds = pollFds;
    for (std::vector<struct pollfd>::const_iterator i = fds.begin(); i != fds.end(); i++)
    {
        std::cout << "fd: " << i->fd << ", events: " << i->events << ", revents: " << i->revents << std::endl;
    }
}