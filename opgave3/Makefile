#Programma
PROG=first-fit
INCL=first-fit.o mem-func.o

PROG2=worst-fit
INCL2=worst-fit.o mem-func.o

SEED=10

#Compiler
CC=gcc
CFLAGS=-Wall -ansi -O3 -Wextra -Werror

#Zoek de systeem architectuur op
ARCH := $(shell getconf LONG_BIT)

#Andere vereiste bestanden die niet gecompileerd kunnen worden
O_FILES=input/mem_test2-$(ARCH).o

O_MEM=mem-access-$(ARCH).o

all: $(PROG) $(PROG2)

#Programma linken
$(PROG): $(O_FILES) $(INCL) $(O_MEM)
	$(CC) $(CFLAGS) -o $(PROG) $^ -lm

$(PROG2): $(O_FILES) $(INCL2) $(O_MEM)
	$(CC) $(CFLAGS) -o $(PROG2) $^ -lm

#Bronbestanden compileren
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#Gecompileerde bestanden opschonen
.PHONY: clean
clean: 
	rm -f *.o $(PROG) $(PROG2) 

#Alle programma's runnen, de uitvoer komt in de out dir
.PHONY: run
run: all
	more input/input.txt | ./$(PROG2) $(SEED) > out/$(PROG2).txt
	more input/input.txt | ./$(PROG) $(SEED) > out/$(PROG).txt
	
#Opschonen uitvoer bestanden
.PHONY: cleanout
cleanout:
	cd out && rm -f *.txt

#Alles opschonen
.PHONY: cleanall
cleanall: cleanout clean

#Setup
.PHONY: setup
setup:
	rm -rf out
	mkdir out
