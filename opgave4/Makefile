#Programma

#~ PROG=fat-extracter
#~ INCL=FAT-12-extracter.o

PROG=fat-checker
INCL=FAT-12-checker.o

FILE=disk-image

#Compiler
CC=gcc
CFLAGS=-Wall -ansi -O2 -Wextra -Werror -g

#Programma linken
$(PROG): $(INCL)
	$(CC) $(CFLAGS) -o $(PROG) $^ -lm
	
#Bronbestanden compileren
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
#Gecompileerde bestanden opschonen
.PHONY: clean
clean: 
	rm -f *.o $(PROG) 
	cd out && rm -f *.txt
	cd extract && rm -rf *

#Alle programma's runnen, de uitvoer komt in de out dir
.PHONY: run
run: $(PROG)
	cd extract && rm -rf *
	for number in 1 2 3 4 5 6; do \
		./$(PROG) $(FILE)$$number > out/out$$number.txt ; \
	done
	
.PHONY: setup
setup:
	mkdir out
	mkdir extract