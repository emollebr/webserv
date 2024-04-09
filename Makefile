NAME := webserv
C := c++
CFLAGS := -g -Wall -Wextra -Werror -std=c++98 
INC:= -I.
SRCS:= main.cpp Server.cpp Request.cpp
OBJS := $(SRCS:.cpp=.o)
##########################################################################

all: $(NAME)

$(NAME): $(OBJS)
	c++ $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	c++ $(CFLAGS) -c $< -o $@

clean:
	   rm -f ${OBJS}

fclean:	clean
	   rm -f ${NAME} ${OBJS}

re: clean all
