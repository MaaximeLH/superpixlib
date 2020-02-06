NAME = theorie
PATH_SRC = ./source/
HEADER = ./include/
FLAG = -std=c11 -Wall -Wextra -Werror -pthread
SRC = $(PATH_SRC)main.c 
OBJ = main.o


all: $(NAME)

$(NAME):
	gcc-8 $(FLAG) -I $(HEADER) $(SRC) -o $(NAME)

clean:
	rm -f $(OBJ)
fclean: clean
	rm -f $(NAME)

re: fclean all
