# C compiler to use
CC = g++

# Compile-time flags
CFLAGS = -Wall -g

# directories to include
INCLUDES = -I./

# source files
SRCS = simple_cross.cpp

# executable file name
MAIN = simple_cross

.PHONY: clean

all:	$(SRCS)
				$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(SRCS)
				@echo  App named simple_cross has been compiled

clean:
			$(RM) *.o *~ $(MAIN)