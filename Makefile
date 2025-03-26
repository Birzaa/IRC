#--------------------Colors---------------------

GREEN           = \033[0;32m
RED             = \033[0;31m
YELLOW          = \033[0;33m
CYAN            = \033[1;36m
MAGENTA         = \033[0;35m
ORANGE          = \033[38;5;216m
NC              = \033[0m

#--------------------Program--------------------

NAME            = irc

#--------------------Compilation----------------

CXX             = c++
CFLAGS          = -Wall -Wextra -Werror -std=c++98 -g3
RM              = rm -rf

#--------------------PATHS----------------------

OBJ_PATH        = .objects

#--------------------Sources--------------------

HEADERS         = $(addsuffix .hpp, \
					Server \
					Client \
					Channel \
					)

SRCS            = $(addsuffix .cpp, \
					main \
					Server \
					Client \
					Channel \
					)

OBJS            = $(SRCS:%.cpp=$(OBJ_PATH)/%.o)

DEPS            = $(OBJS:.o=.d)

#--------------------RULES---------------------

all: $(NAME)

$(NAME): $(OBJS) $(HEADERS)
	@$(CXX) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "${MAGENTA}$@ Compiled successfully${NC}"

$(OBJ_PATH)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJ_PATH)
	@echo "${YELLOW}Finished cleaning${NC}"

fclean: clean
	@$(RM) $(NAME)
	@echo "${YELLOW}Complete cleaning completed${NC}"

re: fclean all

-include $(DEPS)

run: $(NAME)
	@./$(NAME) 6667 123

valgrind: $(NAME)
	@valgrind --leak-check=full ./$(NAME) 6667 123

.PHONY: all clean fclean re