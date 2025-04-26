CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INCDIR)

INCDIR = headers
SRCDIR = srcs
OBJDIR = objs

NAME = webserv

SRCS = $(SRCDIR)/main.cpp \
$(SRCDIR)/json/ft_json.cpp \
$(SRCDIR)/config/config.cpp \
$(SRCDIR)/config/Server.cpp \
$(SRCDIR)/config/LocationData.cpp \
$(SRCDIR)/utils/utils.cpp \

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re