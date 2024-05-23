FLAGS = -Wall -g

.PHONY: all clean run cps

all: ttt mync

# exe:
ttt: ttt.c
	gcc $(FLAGS) $^ -o $@ 

mync: mynetcat.c ttt 
	gcc $(FLAGS) $< -o $@ 

#run:
run_e: mync
	./mync -e "ttt $(s)"

# Cleaning:
clean:
	rm -f ttt mync *.o *.out

# git usage:
cps:
	git commit -a -m "$(m)"
	git push
	git status