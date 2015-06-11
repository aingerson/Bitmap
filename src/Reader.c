#include <stdio.h>
#include "Reader.h"
#include <stdlib.h>


struct bitstring **rows,**cols;
int numRows, numCols, rowBytes, colBytes;

/*
 * Reads a bitmap file and saves as rows and columns by bytes (so 8 bits are stored as one char (1 byte))
 */
void read(char file[]){
	FILE *fp = fopen(file, "r");
	if(fp == NULL){
		fprintf(stderr,"Can't open file\n");
	}
	else{
		int c;
		FILE *fp2 = fopen(file,"r");
		numRows = numCols = 0;
		while((c=getc(fp2)) != EOF){//go through and count how many rows and columns
			if(c=='\n'){
				numRows++;
			}
			if(numRows==0) numCols++;
		}
		numRows++;

		//figure out how many bytes we need for each row and column (round up and then we will fill the rest with 0s)
		rowBytes = bytes(numRows);
		colBytes = bytes(numCols);
		numRows = rowBytes*CHARSIZE;
		numCols = colBytes*CHARSIZE;

		//allocate our pointers to each row and column
		rows = (struct bitstring **) malloc(sizeof(struct bitstring *) * numRows);
		cols = (struct bitstring **) malloc(sizeof(struct bitstring *) * numCols);

		readRows(fp);//parse the rows and then the columns
		readCols();

		//for testing
		printRows();
		printCols();
	}
}

/*
 * Prints the data by rows (for testing)
 */
void printRows(){
	printf("\nPrinting Rows\n");
	int i,j;
	char *toPrint;
	for(i=0; i<numRows; i++){
		toPrint = (rows[i])->bits;

		for(j=0;j<rowBytes;j++){
			showbits(toPrint[j]);
		}
		printf("\n");
	}
}


/*
 * Basically a ceiling function to figure out how many bytes we will need to store the data
 */
int bytes(int n){
	if(n%CHARSIZE == 0){
		return (n/CHARSIZE);
	}
	else{
		n += (CHARSIZE-(n%CHARSIZE));
		return (n/CHARSIZE);
	}
}

/*
 * Prints the data by columns (for testing)
 */
void printCols(){
	printf("\nPrinting Columns\n");
		int i,j;
		char *toPrint;
		for(i=0; i<numCols; i++){
			toPrint = (cols[i])->bits;

			for(j=0;j<colBytes;j++){
				showbits(toPrint[j]);
			}
			printf("\n");
		}
}


/*
 * Parse a file and saves in row format
 */
void readRows(FILE *fp){
	int c,i,j;
	char w;
	char *thisWord = (char *) malloc(CHARSIZE);//the 8 bits (in character format) we are parsing
	char *thisRow = (char *) malloc(rowBytes);//the row we're parsing
	i=0;
	for(j=0; j<numRows; j++){//go through each row
		int b = 0;
		while((c=getc(fp))!='\n' && c!=EOF){//scan this row
			thisWord[i] = c;
			i++;

			if(i==CHARSIZE){//if we have enough data for one byte (8 bits parsed as characters)
				w = readByte(thisWord);//collapse these 8 characters into 1, 8 bit character
				thisRow[b] = w;//and save it
				b++;
				i=0;//reset
			}
		}
		if(i>0){//we get here if we finished parsing the row and need to fill the rest in with 0s
			while(i<CHARSIZE){
				thisWord[i] = '0';//fill the rest with 0s
				i++;
			}
			w = readByte(thisWord);//and then get collapse into one character and save it
			thisRow[b] = w;
			i=0;
		}

		//we've filled in the entire row so we can make a bitstring out of it
		struct bitstring *aRow = (struct bitstring *) malloc(sizeof(struct bitstring));
		char *dest = (char *) malloc(rowBytes);
		copy(dest,thisRow,rowBytes);//copy it into its bitstring data

		aRow -> bits = dest;
		rows[j] = aRow;


		if(c==EOF){//we get here if we finished parsing the entire file but we still need to fill in some rows with 0s (for the columns)
			j++;
			while(j<numRows){
				struct bitstring *empty = (struct bitstring *) malloc(sizeof(struct bitstring));//this is an empty row
				char *empt = (char *) malloc(rowBytes);//with empty data
				empty -> bits = empt;
				rows[j] = empty;
				j++;
			}
		}

	}
	free(thisWord);
	free(thisRow);
}

/*
 * Copies the contents of one bitstring into another
 */
void copy(char *dest, char *src, int n){
	int i=0;
	while(i<n){
		dest[i] = src[i];
		i++;
	}
}

/*
 * Takes 8 characters (of 1s and 0s) and collapses them into one character (one byte) bit by bit
 */
char readByte(char *word){
	char ret;
	int i;
	for(i=0; i<CHARSIZE-1; i++){
		if(word[i]=='0') ret &= 0xFE;//set right bit to 0
		else ret |= 0x1;//set right bit to 1
		ret <<= 1;//shift
	}
	if(word[i]=='1') ret |= 0x1;//for last bit
	return ret;
}

/*
 * Prints one character's bits (for testing)
 */
void showbits(char x){
	int i;
	for(i=7; i>=0; i--){
		(x&(1<<i))?putchar('1'):putchar('0');
	}
}

/**
 * Saves the data in the rows in column format
 */
void readCols(){

	int i,j,k;
	char *thisCol = (char *) malloc(colBytes);

	for(i=0;i<numCols;i++){

		int b = i/CHARSIZE;//which row byte we're on
		int off = CHARSIZE-1-(i%CHARSIZE);//which bit of the row byte (from the right)

		for(j=0;j<colBytes;j++){
			int n = j*CHARSIZE;
			for(k=0;k<CHARSIZE;k++){

				//figure out which character we need from the rows data
				struct bitstring *row = rows[n+k];
				char w = (row->bits)[b];
				//test that character with the offset of which bit we need
				if((w>>off) & 1){
					thisCol[j] |= 0x1;//set the last bit to a 1
				}
				else{
					thisCol[j] &= 0xFE;//set the last bit to a 0
				}
				if(k<CHARSIZE-1) thisCol[j] <<= 1;//shift
			}
		}

		//make a new column bitstring and save it as this column
		struct bitstring *aCol = (struct bitstring *) malloc(sizeof(struct bitstring));
		char *dest = (char *) malloc(colBytes);
		copy(dest,thisCol,colBytes);

		aCol -> bits = dest;
		cols[i] = aCol;

	}
	free(thisCol);
}
