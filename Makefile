CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -I$(INCDIR)
CXX    = c++
INCDIR = headers
SRCDIR = srcs
OBJDIR = objs

NAME = webserv

SRCS = $(SRCDIR)/ft_json.cpp \
	$(SRCDIR)/config.cpp \
	$(SRCDIR)/utils.cpp \
	$(SRCDIR)/autoindex.cpp \
	$(SRCDIR)/http_error.cpp \
	$(SRCDIR)/http_status.cpp \
	$(SRCDIR)/Request.cpp \
	$(SRCDIR)/VirtualServer.cpp \
	$(SRCDIR)/LocationData.cpp \
	$(SRCDIR)/Poller.cpp \
	$(SRCDIR)/AOutputHandler.cpp \
	$(SRCDIR)/AIOHandler.cpp \
	$(SRCDIR)/AInputHandler.cpp \
	$(SRCDIR)/Request.cpp \
	$(SRCDIR)/Response.cpp \
	$(SRCDIR)/CgiHandler.cpp \
	$(SRCDIR)/Connection.cpp \
	$(SRCDIR)/Logger.cpp \
	$(SRCDIR)/Uri.cpp \

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJS) $(OBJDIR)/main.o
	$(CXX) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

tests/%.test: tests/%.cpp $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)
	rm -f tests/*.test
	rm -rf logs/*

re: fclean all

.PHONY: all clean fclean re