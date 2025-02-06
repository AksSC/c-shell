CC = gcc
SRC = src/main.c src/prompt.c src/command.c src/hop.c src/reveal.c src/log.c src/proclore.c src/seek.c src/iMan.c src/neonate.c src/activities.c src/fg_bg.c src/alias.c
OBJ = $(SRC:.c=.o) # Replace .c with .o to create objects
TARGET = my_shell # Executables name

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -D_GNU_SOURCE -g -o $(TARGET) $(OBJ)
# Links objects to make executable

%.o: %.c
	$(CC) $(CFLAGS) -D_GNU_SOURCE -g -c $< -o $@
# To tell how to create objects from source

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean
