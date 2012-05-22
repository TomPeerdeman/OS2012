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
#include <string.h>

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

/* The list of indices that differ in the two FAT tables or are incorrect. */
static unsigned char *badIndices;
/* The list of indices and which file it belongs to. */ 
static dirEntry **indiceTable;

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
void printDirEntry(dirEntry *e){
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
int followDirEntry(dirEntry *e, unsigned short *sFAT){
	int cur =e->start;
	int next;
	int nclusters = 0;
	int size = toLong(e->length);
	int nexpected = 0;
	nexpected = (size + clusterSize - 1) / clusterSize;
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
		cur = next;
	}while(next && (next < 0x0FF0) && (nclusters < nexpected));
	return nclusters;
}

/* This routine will read a file from disk and store it in a buffer. This
 * routine will also check for errors while reading the file.
*/
int bufferFile(dirEntry *e, unsigned short *sFAT, char **buffer){
/* First find number of clusters in file */
	int cur = e->start;
	int nclusters = followDirEntry(e, sFAT);
	int nbytes;
	int nread = 0;
	int prev = cur;
	int offset = 0;
	int inTable = 0;
	dirEntry *copy = NULL;
	
	(*buffer) = NULL;
	if(nclusters == 0) return 0;

	*buffer = calloc(nclusters * bps * spc, 1);
	
	/* Copy the dir entry. When the buffer is overwritten, the dirEntry still
	 * exists. */
	copy = malloc(sizeof(dirEntry));
	memcpy(copy, e, sizeof(dirEntry));
	
	do{
		if(indiceTable[cur] == NULL){
			/* Cluster not yet accessed. */
			inTable = 1;
			indiceTable[cur] = copy;
		}else if(indiceTable[cur] == copy){
			/* Access a cluster which i already have accessed.
			 * There must be a loop. */
			printf("Loop found at index %d\n", cur);
			printDirEntry(copy);
			puts("");
			
			/* Return to prevent infinite looping. */
			(*buffer) = NULL;
			return -2;
		}else if(!memcmp(copy, indiceTable[cur], sizeof(dirEntry))){
			/* Same entrydata but not the same pointer. Entry is
			 * a duplicate. */
			printf("Duplicate file found\n");
			printDirEntry(copy);
			puts("");
			
			/* No need to read the cluster-chain again. */
			(*buffer) = NULL;
			return -2;
		}else{
			/* Cluster is accessed by another file. Two possibilities:
			 * 1. Two directory entries 1 file.
			 * 2. Two directory entries 2 files that merge into one file.
			 */
			printf("Double link found at index %d for file (%p):\n",
				cur, copy);
			printDirEntry(copy);
			printf("The link was also accessed by (%p):\n", 
				indiceTable[cur]);
			printDirEntry(indiceTable[cur]);
			puts("");
		}
	
		/* Notice bad index. */
		if(badIndices[cur]){
			printf("Note: ");
			printf("cluster %d marked as inconsistent. clusters read: %d,"
				"clusters expected: %d\n", cur, nread, nclusters);
			printDirEntry(e);
		}
		
		/* Read the actual cluster into the buffer. */
		lseek(fid, (cur + dataStart) * clusterSize, SEEK_SET);
		nbytes = read(fid, (*buffer) + offset, clusterSize);
		if(clusterSize != nbytes){
			printf("Disk read error, expected %d bytes, read %d\n",
				clusterSize, nbytes);
			printf("Attempting to read cluster %d\n", cur);
			return -1;
		}
		
		nread++;
		offset += clusterSize;
		prev = cur;
		cur = sFAT[cur];
	}while(cur && (cur < 0x0FF0) && (nread < nclusters));
	
	/* If the copy is not used free it. */
	if(!inTable){
		free(copy);
	}
	
	if(!cur){
		if(nread != nclusters){
			printf("Empty marker found in chain at index %d, read %d "
				"clusters. I expected %d clusters.\n", prev, nread, 
				nclusters);
		}else{
			printf("Empty marker found in chain at index %d. I expected"
				" an end of chain marker.\n", prev);
		}
		printDirEntry(e);
		puts("");
		return -2;
	}else if(cur < 0x0FF0){
		/* nread >= nclusters => chainlength > file length */
		printf("Incorrect file length, read %d cluster(s), next cluster"
			" would be at %d but should be an end of chain marker.\n",
			nread, cur);
		printDirEntry(e);
		puts("");
		return -2;
	}else if(nread != nclusters){
		/* nread < nclusters => chainlength < file length */
		printf("Incorrect file length, chain ends at cluster %d, but the"
			"file length gives %d clusters.\n", nread, nclusters);
		printDirEntry(e);
		puts("");
		return -2;
	}
	return nclusters;
}
	
/* Read the entries in a directory (recursively).
   Files are read in, allowing further processing if desired
*/
int readDirectory(dirEntry *dirs, int Nentries, unsigned short *sFAT, int fat){
	int i, j;
	char *buffer = NULL;
	int nclusters = 0;
	for(j = i = 0; i < Nentries; i = j + 1){			
		/*printDirEntry(dirs + i);*/
		for(j = i; j < Nentries; j++){
			if(dirs[j + 1].attrib != 0x0f) break;
			/*printDirEntry(dirs + j); */
		}
		if((dirs[i].name[0] == 0x05) || (dirs[i].name[0] == 0xe5)){
			/*printf("Deleted entry\n");*/
		}else if(dirs[i].name[0] > ' ' && (dirs[i].name[0] != '.')){
			free(buffer);
			nclusters = bufferFile(dirs + i, sFAT, &buffer);
			if(buffer && (dirs[i].attrib & 0x10) && (nclusters > 0)){
				int N;
				/* this must be another directory
				   follow it now */
				N = nclusters * clusterSize / sizeof(dirEntry);
				readDirectory((dirEntry *) buffer, N, sFAT, fat);
			} 
		}
	}
	free(buffer);
	return 0;
}

/* Convert a 12 bit FAT to 16bit short integers
*/
void expandFAT(unsigned char *FAT, unsigned short *sFAT, int entries){
	int i, j;

	for(i = 0, j = 0; i < entries; i += 2, j += 3){
		sFAT[i] = FAT[j] + 256 * (FAT[j + 1] & 0xf);
		sFAT[i + 1] = ((FAT[j + 1] >> 4) & 0xf) + 16 *FAT[j + 2];
	}
}

/* Compare FAT1 to FAT2, inconsistent links ad invalid links are makred as bad
 * and should be checked when accessed. */
void compareFATs(unsigned short *sFAT1, unsigned short *sFAT2, int entries){
	int i;
	
	for(i = 0; i < entries; i++){
		if(sFAT1[i]== 1 || (sFAT1[i] > entries && sFAT1[i] < 0xFF0)){
			printf("Incorrect FAT 1 contents at index %d: %hu\n", i,
				sFAT1[i]);
			badIndices[i] = 1;
		}else if(sFAT2[i]== 1 || (sFAT2[i] > entries && sFAT2[i] < 0xFF0)){
			printf("Incorrect FAT 2 contents at index %d: %hu\n", i,
				sFAT2[i]);
		}
	
		if(sFAT1[i] != sFAT2[i]){
			printf("FAT inconsistency at index %d (fat1/fat2: %hu/%hu)\n",
				i, sFAT1[i], sFAT2[i]);
			badIndices[i] = 1;
		}
	}
}

/* Check not used indices for empty markers.
*/
void checkFreeSpace(unsigned short *sFAT, int entries){
	int i;
	
	for(i = 2; i < entries; i++){
		if(indiceTable[i] == NULL && sFAT[i] != 0){
			if(badIndices[i]){
				printf("Note: cluster %d marked as inconsistent.\n", i);
			}
			printf("Index %d not (correct) used but is marked as non free"
				" in the FAT (%hu)\n", i, sFAT[i]);
		}
	}
}

/* Search files that are not listed in any directory.
*/
void checkLooseFiles(unsigned short *sFAT, int entries){
	unsigned short next;
	int i, j, linkFound, nclusters, entryUsed = 0;
	dirEntry *entry = calloc(1, sizeof(dirEntry));
	unsigned char *buffer = malloc(5);
	
	/* Set name of the entry used for all loose files. */
	entry->name[0] = 'L';
	entry->name[1] = 'O';
	entry->name[2] = 'O';
	entry->name[3] = 'S';
	entry->name[4] = 'E';
	entry->name[5] = ' ';
	entry->name[6] = 'F';
	entry->name[7] = 'I';
	entry->ext[0] = 'L';
	entry->ext[1] = 'E';
	entry->ext[2] = 'S';
	
	for(i = 2; i < entries; i++){
		if(sFAT[i] > 1 && indiceTable[i] == NULL){
			linkFound = 0;
			/* Find first indices. This are the start clusters. */
			for(j = 2; j < entries; j++){
				if(sFAT[j] == i){
					linkFound = 1;
					entryUsed = 1;
					break;
				}
			}
			/* Not a first cluster. */
			if(linkFound)continue;
			
			/* Loose file/dir found starting at index i. Now follow it */
			next = i;
			nclusters = 0;
			while(next && next < 0xFF0){
				/* Set index as 'used'. */
				indiceTable[next] = entry;
				nclusters++;
				next = sFAT[next];
			}

			/* Try to read first 5 bytes of cluster. */
			lseek(fid, (i + dataStart) * clusterSize, SEEK_SET);
			if(read(fid, buffer, 5) != 5){
				perror("Disk error");
			}
			
			/* Empty directory cluster, just ignore. */
			if(buffer[0] == 0xE5
				&& buffer[1] == 0xE5
				&& buffer[2] == 0xE5
				&& buffer[3] == 0xE5
				&& buffer[4] == 0xE5
			){
				continue;
			}
			
			printf("Loose file/directory found at index %d of %d "
				"clusters. Size is between %d and %d bytes.\n", i,
				nclusters, (clusterSize * (nclusters - 1) + 1), 
				(clusterSize * nclusters));

		}
	}
	if(!entryUsed){
		free(entry);
	}
	free(buffer);
}

/* We'll allow for at most two FATs on a floppy, both as 12 bit values and
   also converted to 16 bit values
*/
unsigned char *FAT1;
unsigned char *FAT2;

unsigned short *sFAT1;
unsigned short *sFAT2;

int main(int argc, char * argv[]){
	BPB bootsector;
	int rv = 0;
	int entries;
	int NFATbytes;
	int dataSectors;
	int clusters;
	int i, j;
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
	
	/*expand the FAT's to 16 bit shorts. */
	sFAT1 = calloc(entries + 1, sizeof(unsigned short));
	sFAT2 = calloc(entries + 1, sizeof(unsigned short));
	expandFAT(FAT1, sFAT1, entries);
	expandFAT(FAT2, sFAT2, entries);
	
	i = bps / sizeof(dirEntry);
	NdirSectors = (Ndirs + i - 1) / i;
	dataStart = 1 + (bootsector.NFats * NFATbytes) / bps + NdirSectors - 2;
	dirs = calloc(Ndirs, sizeof(dirEntry));
	printf("dataStart = %d\n", dataStart);

	badIndices = calloc(entries, 1);
	indiceTable = (dirEntry **) calloc(entries, sizeof(dirEntry *));
	
	printf("\nChecking FAT\n");
	compareFATs(sFAT1, sFAT2, entries);
	
	/* Read the root directory. */
	nread = read(fid, dirs, bps * NdirSectors);
	if(nread != bps * NdirSectors){
		printf("Unexpected EOF\n");
		return 1;
	}
	
	printf("\n\nChecking files\n");
	readDirectory(dirs, Ndirs, sFAT1, entries);
	
	printf("\n\nChecking unreferenced files\n");
	checkLooseFiles(sFAT1, entries);
	
	printf("\n\nChecking free space\n");
	checkFreeSpace(sFAT1, entries);
	
	printf("\n\nCleanup\n");
	/* Free dirEntries in indiceTable. */
	for(i = 0; i < entries; i++){
		if(indiceTable[i] != NULL){
			printDirEntry(indiceTable[i]);
			
			free(indiceTable[i]);
		
			/* Dont free array items with the same ptr as the on just free'd
			 * or else the same memory will be free'd twice. */
			for(j = i + 1; j < entries; j++){
				if(indiceTable[j] == indiceTable[i]){
					indiceTable[j] = NULL;
				}
			}
			
			indiceTable[i] = NULL;
		}
	}
	free(indiceTable);
	
	free(badIndices);
	free(dirs);
	free(sFAT1);
	free(sFAT2);
	free(FAT1);
	free(FAT2);
	
	return 0;
}
