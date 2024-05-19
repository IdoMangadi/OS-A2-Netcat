FLAGS = -Wall -g

.PHONY: all clean

all: mync

# exe:
mync: mynetcat_v1.c
	gcc $(FLAGS) $^ -o $@ 

# Cleaning:
clean:
	rm -f mync *.o *.out