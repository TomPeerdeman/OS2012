#Programma

PROG_EXTRACT=fat-extracter
INCL_EXTRACT=FAT-12-extracter.o

PROG_CHECK=fat-checker
INCL_CHECK=FAT-12-checker.o

FILE=disk-image

#Compiler
CC=gcc
CFLAGS=-Wall -ansi -Wextra -Werror -O3

#Alles compileren
.PHONY: all
all: $(PROG_CHECK) $(PROG_EXTRACT)

#Programma linken
$(PROG_CHECK): $(INCL_CHECK)
	$(CC) $(CFLAGS) -o $(PROG_CHECK) $^ -lm
	
$(PROG_EXTRACT): $(INCL_EXTRACT)
	$(CC) $(CFLAGS) -o $(PROG_EXTRACT) $^ -lm
	
#Bronbestanden compileren
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
#Gecompileerde bestanden opschonen
.PHONY: clean
clean: 
	rm -f *.o $(PROG_CHECK) $(PROG_EXTRACT)
	cd out && cd checker && rm -f *.txt
	cd out && cd extracter && rm -f *.txt
	cd extract && rm -rf *

#Alle programma's runnen, de uitvoer komt in de out dir
.PHONY: run
run: all
	make extracter && make checker

#Checker runnen
.PHONY: checker
checker: $(PROG_CHECK)
	for number in 1 2 3 4 5 6; do \
		./$(PROG_CHECK) $(FILE)$$number > out/checker/out$$number.txt ; \
	done
	
#Extracter runnen
.PHONY: extracter	
extracter: $(PROG_CHECK)
	cd extract && rm -rf *
	for number in 1 2 3 4 5 6; do \
		./$(PROG_EXTRACT) $(FILE)$$number > out/extracter/out$$number.txt ; \
	done
	
#Setup
.PHONY: setup
setup:
	rm -rf extract
	rm -rf out
	mkdir out
	cd out && mkdir checker && mkdir extracter
	mkdir extract