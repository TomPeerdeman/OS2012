/* In this file a simple simulated FAT12 type file system is defined.
   The relevant structures in this file system will be:
   1) The disk information block BPB, describing the properties of the
	  other structures.
   2) The FATs
   3) The root directory. This directory must be present.
   4) Data blocks
   All structures will be allocated in units of 512 bytes, corresponding
   to disk sectors.
   Data blocks are allocate in clusters of spc sectors. The FATs refer
   to clusters, not to sectors.
   The BPB below was derived from the information provided on the
   Atari TOS floppy format; it is also valid for MS DOS floppies,
   but Atari TOS allows more freedom in choosing various values.

   A start has been made to incorporate VFAT long file names (LFN),
   but this was not completed.
   Error checking is barely present - incorporating this is part
   of an OS assignment.

   G.D. van Albada
   (c) IvI, Universiteit van Amsterdam, 2012
   
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* The following structure describes the boot block - the first 512 bytes on
   the floppy disk
*/
typedef struct BPB{
   unsigned short BRA;		 /* Branch to boot code		   */
   unsigned short filler[3];   /* reserved for OEM			  */
   unsigned char  vsn[3];	  /* volume-serial number 24 bit   */
   unsigned char  bps[2];	  /* bytes per sector			  */
   unsigned char  spc;		 /* sectors per cluster		   */
   unsigned char  res[2];	  /* reserved					  */
   unsigned char  NFats;	   /* number of FATs				*/
   unsigned char  Ndirs[2];	/* number of directory entries   */
   unsigned char  Nsects[2];   /* number of sectors on media	*/
   unsigned char  Media;	   /* Media descriptor			  */
   unsigned char  spf[2];	  /* Sectors per FAT			   */
   unsigned char  spt[2];	  /* Sectors per track			 */
   unsigned char  Nsides[2];   /* Sides on media				*/
   unsigned char  Nhid[2];	 /* hidden sectors				*/
   unsigned char  boot[482];   /* boot code					 */
} BPB;

/* The following structure describes a directory entry.
   The definition is only valid on little-endian systems,
   due to the use of shorts.
*/
typedef struct DIRENTRY{
	unsigned char name[8];
	unsigned char ext[3];
	unsigned char attrib;
	unsigned char zero[10];
	unsigned short time;
	unsigned short date;
	unsigned short start;
	unsigned short length[2];
} dirEntry;

/* The following are default values. Most are recomputed on basis of the 
   information in the bootblock.
*/
static int clusterSize = 1024;
static int bps = 512;
static int spc = 2;
static int dataStart = 0;

/* The file identified for the input file is stored here, as a global identifier
*/
static int fid;

/* Two macros to help convert bytes to short values and short values
   to 4 byte integers
*/
#define toShort(b) ((b[0] & 0xff) + 256 * (b[1] & 0xff))
#define toLong(b) ((b[0] & 0xffff) + (((long)(b[1] & 0xffff)) << 16))

/* print a directory entry
*/
void printDirEntry(dirEntry * e){
	int i;
	short *zero = (short *) e->zero;
	if(e->attrib == 0x0f){
		printf("LFN:");
		for(i = 1; i < 32; i++){
			if(e->name[i] >= ' ') printf("%c", e->name[i]);
			else if(e->name[i]) printf("&%x;", e->name[i]);
		}
		printf("\n");
	}else if(e->name[0] != 0){
		for(i = 0; i < 8; i++){
			if(e->name[i] < ' '){
				break;
			}
			printf("%c", e->name[i]);
		}
		for(i = 0; i < 3; i++){
			if(e->ext[i] < ' '){
				break;
			}
			printf("%c", e->ext[i]);
		}
		printf(" (%x)", e->attrib);
		for(i = 0; i < 5; i++){
			printf(" %hu", zero[i]);
		}
		printf(" time: %hu date: %hu start: %hu length: %ld\n", e->time,
			e->date, e->start, toLong(e->length));
	}
}

/* The following code should obtain the total actual number of clusters
   in a chain starting at some directory-entry.start
*/
int followDirEntry(dirEntry *e, unsigned short * sFAT){
	int cur =e->start;
	int next;
	int nclusters = 0;
	int size = toLong(e->length);
	int nexpected = 0;
	nexpected = (size + clusterSize - 1) / clusterSize;
	printf("Following chain from %d\n", cur);
	/* For directories a length of zero is generally specified.
	   This code will thus read only a single cluster for directories. */
	if(e->attrib == 0x0f){
		return 0;
	}
	if(e->attrib & 0x08){
		printf("Volume label, not a file\n");
		return 0;
	}
	if(cur < 2){
		printf("Not a valid starting cluster\n");
		return 0;
	}
	
	do{
		nclusters++;
		next = sFAT[cur];
		printf("%d ", next);
		cur = next;
	}while(next && (next < 0x0FF0) && (nclusters < nexpected));
	printf("\nNclusters = %d\n", nclusters);
	return nclusters;
}

/* This routine will read a file from disk and store it in a buffer
*/
int bufferFile(dirEntry *e, unsigned short * sFAT, char ** buffer){
/* First find number of clusters in file */
	int cur = e->start;
	int nclusters = followDirEntry(e, sFAT);
	int nbytes;
	int nread = 0;
	int next;
	int offset = 0;
	(*buffer) = NULL;
	if(nclusters == 0) return 0;

	*buffer = calloc(nclusters * bps * spc, 1);
	do{
		lseek(fid, (cur + dataStart) * clusterSize, SEEK_SET);
		if(clusterSize != (nbytes = read(fid, (*buffer) + offset, clusterSize))){
			printf("Disk read error, expected %d bytes, read %d\n", clusterSize, nbytes);
			printf("Attempting to read cluster %d\n", cur);
			return -1;
		}
		nread++;
		offset += clusterSize;
		next = sFAT[cur];
		cur = next;
	}while(next && (next < 0x0FF0) && (nread < nclusters));
	if(next < 0x0FF0){
	/* not a normal end of chain */
		printf("Broken file, read %d clusters, expected %d, next cluster would be at %d\n",
				nread, nclusters, next);
		return -2;
	}
	return nclusters;
}
	
/* Read the entries in a directory (recursively).
   Files are read in, allowing further processing if desired
*/
int readDirectory(dirEntry *dirs, int Nentries, unsigned short * sFAT){
	int i, j;
	char * buffer = NULL;
	int nclusters = 0;
	
	for(j = i = 0; i < Nentries; i = j + 1){
		printDirEntry(dirs + i);
		for(j = i; j < Nentries; j++){
			if(dirs[j + 1].attrib != 0x0f) break;
			printDirEntry(dirs + j);
		}
		if((dirs[i].name[0] == 0x05) || (dirs[i].name[0] == 0xe5)){
			printf("Deleted entry\n");
		}else if(dirs[i].name[0] > ' ' && (dirs[i].name[0] != '.')){
			free(buffer);
			nclusters = bufferFile(dirs + i, sFAT, &buffer);
			if(buffer && (dirs[i].attrib & 0x10) && (nclusters > 0)){
				int N;
				/* this must be another directory
				   follow it now */
				for(N = 0; N < 8; N++){
					if(dirs[i].name[N] == '\0'){
						break;
					}
					if(dirs[i].name[N] == ' '){
						dirs[i].name[N] = '\0';
						break;
					}
				}
				printf("I will write the dir %s\n", dirs[i].name);

				if(mkdir((char *)dirs[i].name, 0777)){
					perror("Mkdir");
				}
				
				if(chdir((char *)dirs[i].name)){
					perror("Chdir");
				}
				
				N = nclusters * clusterSize / sizeof(dirEntry);
				printf("Reading directory\n");
				readDirectory((dirEntry *) buffer, N, sFAT);
				
				if(chdir("../")){
					perror("Chdir");
				}
			}else if(buffer && nclusters > 0){
				char buf[13];
				int start = 0;
				int c;
				FILE *file;
				
				while(start < 8){
					if(dirs[i].name[start] == ' ')break;
					buf[start] = dirs[i].name[start];
					start++;
					
				}
				
				buf[start++] = '.';
				
				for(c = 0; c < 3; c++){
					buf[start] = dirs[i].ext[c];
					start++;
				}
				
				buf[start] = '\0';
				
				printf("\tI will write the file %s; length: %ld\n", buf, toLong(dirs[i].length));
				
				file = fopen(buf, "w+");
				if(fwrite(buffer, 1, toLong(dirs[i].length), file) != (unsigned long) toLong(dirs[i].length)){
					printf("Error: write error\n");
				}
				fclose(file);
			}			
		}

	}
	free(buffer);
	return 0;
}

/* Convert a 12 bit FAT to 16bit short integers
*/
void expandFAT(unsigned char * FAT, unsigned short * sFAT, int entries){
	int i;
	int j;
	for(i = 0, j = 0; i < entries; i += 2, j += 3){
		sFAT[i] = FAT[j] + 256 * (FAT[j + 1] & 0xf);
		sFAT[i + 1] = ((FAT[j + 1] >> 4) & 0xf) + 16 * FAT[j + 2];
	}
}

/* Convert a FAT represented as 16 bit shorts back to 12 bits to
   allow rewriting the FAT
*/
void shrinkFAT(unsigned char * FAT, unsigned short * sFAT, int entries){
	int i;
	int j;
	for(i = 0, j = 0; i < entries; i += 2, j += 3){
		FAT[j] = sFAT[i] & 0xff;
		FAT[j + 1] = ((sFAT[i] & 0x0f00) >> 8) + ((sFAT[i + 1] & 0x000f) << 4);
		FAT[j + 2] = (sFAT[i + 1] & 0x0ff0) >> 4;
	}
}

/* We'll allow for at most two FATs on a floppy, both as 12 bit values and
   also converted to 16 bit values
*/
unsigned char * FAT1;
unsigned char * FAT2;

unsigned short * sFAT1;
unsigned short * sFAT2;

int main(int argc, char * argv[]){
	BPB bootsector;
	int rv = 0;
	int entries;
	int NFATbytes;
	int dataSectors;
	int clusters;
	int i;
	int bps;
	int Ndirs;
	int NdirSectors;
	int nread;
	dirEntry *dirs;
	
	printf("size of bootsector = %u\n", sizeof(BPB));
	if(argc > 1){
		fid = open(argv[1], O_RDONLY);
	}else{
		printf("Need one file argument\n");
		exit(-1);
	}
	
		if(chdir("extract")){
		perror("Chdir");
	}
	
	if(mkdir(argv[1], 0777)){
		perror("Mkdir");
	}
	
	if(chdir(argv[1])){
		perror("Chdir");
	}
	
	if((sizeof(BPB) != (rv = read(fid, &bootsector, sizeof(BPB))))){
		printf("Read error %d\n", rv);
		exit(-2);
	}
	printf("Serial nr. %u-%u-%u\n", bootsector.vsn[0], bootsector.vsn[1], bootsector.vsn[2]);
	printf("bps = %hu\n", bps = toShort(bootsector.bps));
	printf("spc = %hu\n", spc = bootsector.spc);
	printf("res = %hu\n", toShort(bootsector.res));
	printf("NFats = %hu\n", bootsector.NFats);
	printf("Ndirs = %hu\n", Ndirs = toShort(bootsector.Ndirs));
	printf("Nsects = %hu\n", toShort(bootsector.Nsects));
	printf("Media = %hu\n", bootsector.Media);
	printf("spf = %hu\n", toShort(bootsector.spf));
	printf("spt = %hu\n", toShort(bootsector.spt));
	printf("Nsides = %hu\n", toShort(bootsector.Nsides));
	printf("Nhid = %hu\n", toShort(bootsector.Nhid));
	if(!bps || !bootsector.NFats  || !toShort(bootsector.Nsects)){
		printf("This is not a FAT-12-type floppy format\n");
		exit(-2);
	}
	/* Compute the number of entries in the FAT
	   The number of bytes is BPS * spf
	   The maximum number of entries is 2/3 of that.
	   On the other hand, we have a maximum of x data-sectors,
	   where x = Nsects - 1 - NFats * spf
	   or mayby x = Nsects - res
	*/
	NFATbytes = toShort(bootsector.bps) * toShort(bootsector.spf);
	entries = (2 * NFATbytes) / 3;
	dataSectors = toShort(bootsector.Nsects) - 1 /* boot */ - bootsector.NFats * toShort(bootsector.spf);
	clusters = dataSectors / bootsector.spc;
	clusterSize = spc * bps;
	printf("entries = %u, dataSectors = %u, clusters = %u\n", entries, dataSectors, clusters);
	FAT1 = malloc(NFATbytes);
	FAT2 = malloc(NFATbytes);
	nread = read(fid, FAT1, NFATbytes);
	if(nread != NFATbytes){
		printf("Unexpected EOF\n");
	}
	for(i = 1; i < bootsector.NFats; i++){
		nread = read(fid, FAT2, NFATbytes);
		if(nread != NFATbytes){
			printf("Unexpected EOF\n");
		}
	}
	sFAT1 = calloc(entries + 1, sizeof(unsigned short));
	sFAT2 = calloc(entries + 1, sizeof(unsigned short));
	expandFAT(FAT1, sFAT1, entries);
	expandFAT(FAT2, sFAT2, entries);
	printf("FAT1: %hu  %hu  %hu  %hu  %hu  %hu\n", sFAT1[0],
			sFAT1[1], sFAT1[2], sFAT1[3], sFAT1[4], sFAT1[5]);
	printf("FAT2: %hu  %hu  %hu  %hu  %hu  %hu\n", sFAT2[0],
			sFAT2[1], sFAT2[2], sFAT2[3], sFAT2[4], sFAT2[5]);
	
	i = bps / sizeof(dirEntry);
	NdirSectors = (Ndirs + i - 1) / i;
	dataStart = 1 + (bootsector.NFats * NFATbytes) / bps + NdirSectors - 2;
	dirs = calloc(Ndirs, sizeof(dirEntry));
	printf("dataStart = %d\n", dataStart);
	nread = read(fid, dirs, bps * NdirSectors);
	if(nread != bps * NdirSectors){
		printf("Unexpected EOF\n");
	}
	readDirectory(dirs, Ndirs, sFAT1);
	
	
	return 0;
}
