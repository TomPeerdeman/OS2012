#Programma
PROG=first-fit
INCL=first-fit.o mem-func.o

PROG2=worst-fit
INCL2=worst-fit.o mem-func.o

SEED=10

#Compiler
CC=gcc
CFLAGS=-Wall -ansi -O2 -Wextra -Werror

#Andere vereiste bestanden die niet gecompileerd kunnen worden
O_FILES=input/mem_test2-32.o
#O_FILES=input/mem_test2-64.o

#all: $(PROG)
all: $(PROG) $(PROG2)

#Programma linken
$(PROG): $(O_FILES) $(INCL)
	$(CC) $(CFLAGS) -o $(PROG) $^ -lm

$(PROG2): $(O_FILES) $(INCL2)
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