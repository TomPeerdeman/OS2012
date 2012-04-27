/* Dit is de mem_alloc.h file zoals beloofd in de practicumhandleiding */
/*                                                                     */
/*   Auteur:	Dick van Albada                                        */
/*		Faculteit Wiskunde en Informatica		       */
/*		Universiteit van Amsterdam			       */
/*   Datum:	5 oktober 1993                                         */
/*   Versie:	0.01                                                   */
/* 								       */

/* Constanten */

#define MEM_SIZE	(32768)

/* Procedures */

void mem_init(long mem[MEM_SIZE]);

/* mem_init wordt aangeroepen voor enige andere procedure uit deze file
   wordt gebruikt. Hij initialseert de memory-manager en zorgt ervoor
   dat het te beheren geheugen daar bekend is
   */

long  mem_get(long request);

/* mem_get alloceert een stuk geheugen ter grootte van request - indien
   mogelijk en geeft de index van het eerste element van dat stuk geheugen
   terug.
   Indien de aanvraag niet gehonoreerd kan worden, wordt -1 teruggegeven
   */

void mem_free(long index);

/* mem_free wordt aangeroepen om een eerder verkregen stuk geheugen op
   locatie index weer vrij te geven. Index moet een geldige waarde
   hebben
   */

double mem_internal();

/* mem_internal berekent de fractie interne fragmentatie volgens de
   in de handleiding gegeven formule
   */

void mem_available(long *empty, long *large, long *n_holes);

/* mem_available vertelt de gebruiker hoeveel geheugen er nog
   beschikbaar is
   empty:	totale hoeveelheid vrije ruimte
   large:	omvang van het grootste gat, gecorrigeerd voor
		administratie
   n_holes:	het aantal gaten
   */

void mem_exit();

/* mem_exit wordt alleen als laatste routine aangeroepen en beeindigt
   het gebruik van de memory manager.
   Deze routine zorgt zonodig voor opruimen en afronden.
   */
