//to read content from keyboard and write it to a file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	FILE *f1;
	char s[100];
	f1=fopen("write5.txt","w");
	if(f1==NULL)
	{
		printf("The file cannot be opened");
		exit(1);
	}
	
	printf("Enter few lines:\n");
	while((strlen(gets(s)))>0)
	{
		fputs(s,f1);
		fputs("\n",f1);
	}
	fclose(f1);
}
