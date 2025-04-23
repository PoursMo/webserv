CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = webserv

# all: $(NAME)

# $(NAME): $(OBJ)
# 	$(CC) $(CFLAGS) -o $@ $^

# $(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
# 	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# $(OBJDIR):
# 	mkdir -p $(OBJDIR)

# clean:
# 	rm -rf $(OBJDIR)

# fclean: clean
# 	rm -f $(NAME)

# re: fclean all

test:
	$(CC) $(CFLAGS) json/json.cpp -o test

.PHONY: all clean fclean re test