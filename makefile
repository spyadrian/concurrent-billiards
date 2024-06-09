GCC=gcc -Wall -std=gnu11 -g
LIBS=-L. -lcalc_collision_speeds -lSDL2 -lSDL2_image -lSDL2_mixer -lpthread -lm

BIN=billard
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
HEADERS=$(wildcard *.h)

$(BIN): $(OBJS)
	$(GCC) $^ -o $@ $(LIBS)

run: $(BIN)
	./$< 

%.o: %.c $(HEADERS)
	$(GCC) $< -c

clean:
	rm -f $(BIN) *.o
	


