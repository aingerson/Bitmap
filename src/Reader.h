#define CHARSIZE 8

//helps represent a row or column of data
struct bitstring{
	char *bits;
};

void read(char *);
void readRows(FILE *);
void readCols();
char readByte(char *);
void showbits(char);
void printRows();
void printCols();
void copy(char *, char *,int);
int bytes(int);
