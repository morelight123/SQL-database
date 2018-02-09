#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define MAXFIELDS 100 // for now
#define MAXINPUTLENGTH 5000
#define MAXLENOFFIELDNAMES 50
#define MAXLENOFFIELDTYPES 50
#define MAXTABLES 10

char *trimwhitespace(char *str)
{
	char *end;
	while (isspace((unsigned char)*str)) str++;
	if (*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;
	*(end + 1) = 0;
	return str;
}

int main() {
	static char buffer[MAXINPUTLENGTH];
	memset(buffer, 0, MAXINPUTLENGTH);
	char *status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	char *token=malloc(50);
	printf("%-10s%-10s%-10s%-10s%-15s%-10s\n","start","end", "date","day","department","role");
	printf("_____________________________________________________________\n");
	while (status != NULL) {
		trimwhitespace(buffer);
		if (strlen(buffer) < 2 )
			break; // not a real command, CR/LF, extra line, etc.
		if(strncmp(buffer,"===>",4)!=0 &&strncmp(buffer,"Welcome",7)!=0&&strncmp(buffer,"Goodbye",7)!=0) {
			token=strtok(buffer,",");
			printf("%-10s",token);
			token=strtok(NULL,",");
			printf("%-10s",token);
			token=strtok(NULL,",");
			printf("%-10s",token);
			token=strtok(NULL,",");
			printf("%-10s",token);
			token=strtok(NULL,",");
			printf("%-15s",token);
			token=strtok(NULL,"\n");
			printf("%-10s\n",token);
		}
		status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	}
}
