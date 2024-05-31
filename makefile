FLAGS = -Wall -g --coverage

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

run_coverage: mync
	./ttt > /dev/null 2>&1 || true
	./ttt 12345678a > /dev/null 2>&1 || true
	echo "" | ./ttt 123456789 > /dev/null 2>&1 || true
	./mync -e "ttt 123456789" < inputs/mync_win_row_scenario.txt > /dev/null || true
	./mync -e "ttt 123456789" < inputs/mync_win_column_scenario.txt > /dev/null || true
	./mync -e "ttt 123456789" < inputs/mync_lose_diagonal1_scenario.txt > /dev/null || true
	./mync -e "ttt 234567891" < inputs/mync_lose_diagonal2_scenario.txt > /dev/null || true
	./mync -e "ttt 123456789" < inputs/mync_draw_scenario.txt > /dev/null || true
	./mync -e "ttt 123456789" < inputs/mync_invalid_scenario.txt > /dev/null || true
	./mync -e "ttt 123456789" -b TCPS< inputs/mync_invalid2_scenario.txt > /dev/null || true

gcov_analysis:
	gcov ttt.c
	gcov mync-mynetcat.c

# Cleaning:
clean:
	rm -f ttt mync *.o *.out

# git usage:
cps:
	git commit -a -m "$(m)"
	git push
	git status