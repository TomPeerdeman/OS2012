#Programma
PROG=fat-reader
INCL=FAT-12-reader.o

#Compiler
CC=gcc
CFLAGS=-Wall -ansi -O2 -Wextra -Werror



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

#Alle programma's runnen, de uitvoer komt in de out dir
.PHONY: run
run: $(PROG)
	./$(PROG) disk-image3 > out.txt
	