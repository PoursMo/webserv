CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCDIR)

INCDIR = headers
SRCDIR = srcs
OBJDIR = objs

NAME = webserv

SRCS = $(SRCDIR)/ft_json.cpp \
	$(SRCDIR)/config.cpp \
	$(SRCDIR)/VirtualServer.cpp \
	$(SRCDIR)/LocationData.cpp \
	$(SRCDIR)/utils.cpp \

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
	rm -f tests/**/*.test

re: fclean all

.PHONY: all clean fclean re