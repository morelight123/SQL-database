#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define MAXFIELDS 100 // for now
#define MAXINPUTLENGTH 5000
#define MAXLENOFFIELDNAMES 50
#define MAXLENOFFIELDTYPES 50
#define MAXTABLES 10
struct _field {
	char fieldName[MAXLENOFFIELDNAMES];
	char fieldType[MAXLENOFFIELDTYPES];
	int fieldLength;
};
struct _table {
	char *tableFileName;
	char *schemaName;
	int reclen;
	int fieldcount;
	struct _field fields[MAXFIELDS];

};
struct _whereField {
   struct _field fields[MAXFIELDS];
};
struct _multTable {
   struct _table tables[MAXTABLES];
};
typedef enum { false, true } bool;


//pre-declare of some function
bool WHEREandAND(struct _table *table,char *buffer,char *status,struct _whereField *whereField,struct _multTable *multTable,struct _table *dest, int sign[]);
void processCommand(char buffer[MAXINPUTLENGTH]);
void fromMultTable(char *token,struct _multTable *multTable);

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


bool loadDatabase(struct _table *table,char temp[MAXINPUTLENGTH]) {   
// READ THE DATA FROM STDIN AS THE DESIGN OF THE DATABASE 
	char *token;
	int index = 0;
	int i = 0;
	FILE *pfile = fopen(table->tableFileName, "ab");

	char *buffer = calloc(1, table->reclen);
	if (temp == NULL) {  //get every line of stdin
		free(buffer);
        return 0;
	}
	trimwhitespace(temp);
	if (temp[0] == '\n') {
		free(buffer);
        	return 0;
	}
	index = 0;
	i = 0;
	token = strtok(temp, ",");

	while (1) {
		if (i >= table->fieldcount) {
			break;
		}
		if (token != NULL) {
			if (token[strlen(token) - 1] == '\n') {
				strncpy(token, token, strlen(token) - 1);
			}
			if (strlen(token)>table->fields[i].fieldLength) {
				printf("*** WARNING: Data in field %s is being truncated ***\n", table->fields[i].fieldName);
				strncpy(&buffer[index], token, table->fields[i].fieldLength - 1);
			}
			else {
				strncpy(&buffer[index], token, strlen(token));
			}
			index = index + table->fields[i].fieldLength;

			++i;
		}
		else {
			printf("token is NULL\n");
			break;
		}
		if (i >= table->fieldcount - 1) {
			token = strtok(NULL, "\n");
		}
		else {
			token = strtok(NULL, ",");
		}
	}
		/*printf("%s\n",buffer);
		printf("%s\n",&buffer[50]);
		printf("%s\n",&buffer[100]);
		printf("%s\n",&buffer[150]);
		printf("%s\n",&buffer[200]);
		printf("%s\n",&buffer[250]);
		printf("%s\n",&buffer[300]);
		printf("%s\n",&buffer[320]);*/
	if (fwrite(buffer, table->reclen, 1, pfile) != 1) {
		printf("file write error\n");
		return 0;
	}
	free(buffer);

	fclose(pfile);
	return 1;

}


bool loadSchema(struct _table *table) {
   FILE *schemaFile = fopen(table->schemaName, "r");
   table->reclen=0;
   char *t;
	char *temp=malloc(MAXINPUTLENGTH);
   char *token=malloc(MAXINPUTLENGTH);
	int i=0;
	while (1) {
		if (fgets(temp, 998, schemaFile) == NULL) {  //get every line of stdin
			break;
		}
		if (temp[0] == '\n') {
			break;
		}

     token = strtok(temp, " ");
   if(strncmp(token,"END",3)==0) {
        break;}
     if (strncmp(token,"ADD",3)==0) {
        token = strtok(NULL, " ");
        strcpy(table->fields[i].fieldName,token);
        token = strtok(NULL, " ");
        strcpy(table->fields[i].fieldType,token);
        token = strtok(NULL, "\n");
        table->fields[i].fieldLength=atoi(token);
        table->reclen=table->reclen+table->fields[i].fieldLength;
        i++;
     }
       else{
          break;
       }
      
     }
		table->fieldcount=i;
		fclose(schemaFile);
   return 1;
}  


bool getRecord(int recnum, char *record, struct _table *table){   // no need to modify
   //printf("Enter getrecord\n");
   FILE *pfile = fopen(table->tableFileName, "rb");
   //printf("*** LOG: Getting record %d from the database ***\n",recnum);
   //printf("recnum: %d   reclen: %d   %d\n", recnum,table->reclen,recnum*table->reclen);
	fseek(pfile, recnum*table->reclen, SEEK_SET);
   if (fread(record,table->reclen, 1, pfile) != 1) {
		printf("getRecord file read error\n");
		return 0;
	}
   //printf("%s\n",record);
   fclose(pfile);
	return 1;
   
   
}




int CreateTable(struct _table *table) {
	char buffer[MAXINPUTLENGTH];
	memset(buffer, 0, MAXINPUTLENGTH);
	FILE *binFile = fopen(table->tableFileName, "wb");
	FILE *schemaFile = fopen(table->schemaName, "w");
	fclose(binFile);
	fclose(schemaFile);
	char *status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	FILE *schema = fopen(table->schemaName, "a");
	while (status != NULL) {
		trimwhitespace(buffer);
		if (strlen(buffer) < 5 && strncmp(buffer, "END", 3) == 0){
           printf("===> %s\n", buffer);
			break;}
     	printf("===> %s\n", buffer);
		buffer[strlen(buffer)] = '\n';
		if (fwrite(buffer, strlen(buffer), 1, schema) != 1) {
			printf("Create table file write error\n");
			return 0;
		}
		status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	}

	fclose(schema);
   //free(status);
 
}


bool compare(char *value1, char *value2, int sign) {
   int v1=atoi(value1);
   int v2=atoi(value2);
   if(sign==-1) {     //sign is '<='
      if(v1<=v2)
         return true;
      else
         return false;
   }
   else if(sign==1) {     //sign is '>='
      if(v1>=v2)
         return true;
      else
         return false;
   }
   else {
      printf("incorrect sign value\n");
      return false;
   }
}

void showRecord(struct _field *fields, char *record, int fieldcount, char *field, struct _whereField *whereField, int sign[]) {
	
	int index = 0;
	int i = 0;
   int j=0;
	char *temp = calloc(10 * MAXLENOFFIELDNAMES, sizeof(char));
	char *token = calloc(MAXLENOFFIELDNAMES, sizeof(char));
	strcpy(temp, field);
   
//check if the record is the right record judging by wherefield. If no, return, if yes, continue   
   while(strncmp(whereField->fields[j].fieldName,"0",1)!=0 ) {
   		while(i<fieldcount) {
      		if (strncmp(fields[i].fieldName, whereField->fields[j].fieldName, strlen(fields[i].fieldName)) == 0) { 
               //printf("%d: value: %s; Ovalue: %s; index: %d\n",j,whereField->fields[j].fieldType,&record[index],index);
					if(sign[j]==0) {
         			if (strncmp(&record[index], whereField->fields[j].fieldType, strlen(whereField->fields[j].fieldType))==0) {
                  //printf("value: %s\n",&record[index]);
        		 	}
						else{
                 	//printf("value in else: %s\n",&record[index]);
                 	return;
                }
					}
					else {
						if(!(compare(&record[index], &whereField->fields[j].fieldType, sign[j])))
							return;
					}
				}
        index = index + fields[i].fieldLength,


			++i;
   	  		}
      ++j;
      i=0;
      index=0;
   		}
   index=0;
   i=0;
   //address case that select statement without space
   if(strchr(temp,',')){
      token=strtok(temp,",");}
   else{
   		token = strtok(temp, " ");}
   while (strncmp(token, ";", 1) != 0) {
   		while (i<fieldcount) {
			if (strncmp(fields[i].fieldName, token, strlen(fields[i].fieldName)) == 0 &&strncmp(fields[i].fieldName, token, strlen(token)) == 0 ) { //compare required field with existing field
				trimwhitespace(&record[index]);
               if(strncmp(&record[index],"\n",1)!=0)
							printf("%s", &record[index]);
			}
        index = index + fields[i].fieldLength;
			++i;

		}
     token = strtok(NULL, " ");
      if(strncmp(token, ";", 1) != 0)
         printf(",");
      i=0;
      index=0;
	}
	printf("\n");
   //free(token);
   //free(temp);
}


int loadTableForCombine(struct _table *table, char* column) {
   int index=0;
   int x=0;
   if(table->tableFileName[0]==' ') {
			char *temp=malloc(sizeof(table->tableFileName));
			strcpy(temp,&table->tableFileName[1]);
			strcpy(table->tableFileName,temp);
   }
	if(table->schemaName[0]==' ') {
			char *temp=malloc(sizeof(table->schemaName));
			strcpy(temp,&table->schemaName[1]);
			strcpy(table->schemaName,temp);
   }
   if(!loadSchema(table)){
   		printf("%s does not exist in function: loadTable.\n", table->schemaName);
   }
   while(x<table->fieldcount) {
      if(strncmp(table->fields[x].fieldName,column,strlen(table->fields[x].fieldName))==0) {
         //printf("table1->fields[%d].fieldName: %s\n",x1,table1->fields[x1].fieldName);
         break;
      }
      index = index + table->fields[x].fieldLength;
		 ++x;
   }
   return index;
}
bool combineSchema(struct _table *table1, struct _table *table2, struct _table *dest, char *a) {
   //define dest Name
   dest->tableFileName=calloc(1,strlen("newTable0.bin"));
   dest->schemaName=calloc(1,strlen("newTable0.schema"));
   strcpy(dest->tableFileName,"newTable");
   strcpy(dest->schemaName,"newTable");
   strcat(dest->tableFileName,a);
   strcat(dest->tableFileName,".bin");
   strcat(dest->schemaName,a);
   strcat(dest->schemaName,".schema");
//printf("dest->tableFileName: %s\n",dest->tableFileName);
//printf("dest->schemaName: %s\n",dest->schemaName);
   
   //pre-allocate dest space
   FILE *pFile = fopen(dest->schemaName, "w");
   fclose(pFile);
   FILE *binFile = fopen(dest->tableFileName, "wb");
   rewind(binFile);
   fclose(binFile);
   
//start combine schema file
   
   char *status=malloc(sizeof(char));
   char buffer[MAXINPUTLENGTH];
   FILE *writeFile = fopen(dest->schemaName, "a");
   FILE *readFile1 = fopen(table1->schemaName, "r");
   FILE *readFile2 = fopen(table2->schemaName, "r");
//combin two schema file to combin two table
   
 
   while(status = fgets(buffer, MAXINPUTLENGTH - 1, readFile1)) { //append everyline from first schema to new schema
      if (fwrite(buffer, strlen(buffer), 1, writeFile) != 1) {
			printf("Combine table 1 file write error: %s\n",dest->schemaName);
			return 0;
		}
   }  
   while(status = fgets(buffer, MAXINPUTLENGTH - 1, readFile2)) {//append everyline from second schema to new schema
      
         if (fwrite(buffer, strlen(buffer), 1, writeFile) != 1) {
			printf("Combine table 2 file write error: %s\n",dest->schemaName);
			return 0;
		}
   }
   fclose(readFile1);
   fclose(readFile2);
   rewind(writeFile);
   fclose(writeFile);
   //free(status);
   return 1;
}


int getFileSize(struct _table *table) {
   int SizeofFile;
   FILE *bin = fopen(table->tableFileName, "rb");
   fseek(bin, 0L, 2);
   SizeofFile = ftell(bin);
   rewind(bin);
   fclose(bin);
   return SizeofFile;
}

bool writeNewTableBinFile(struct _table *table1, struct _table *table2, struct _table *dest, int index1, int index2) {
   int SizeofFile1=getFileSize(table1);
   int SizeofFile2=getFileSize(table2);
   FILE *writebin = fopen(dest->tableFileName, "ab");
   
   char *record1 = calloc(1, table1->reclen);
   char *record2 = calloc(1, table2->reclen);
   int j = 0;
   int k = 0;
   	for (j = 0; j<SizeofFile1 / table1->reclen; ++j) { //loop record
   		for(k = 0; k<SizeofFile2 / table2->reclen; ++k) {
      			if (getRecord(j, record1, table1)) {
      				if(getRecord(k, record2, table2)) {
            				if(strcmp(&record1[index1],&record2[index2])==0) {
                    				if (fwrite(record1, table1->reclen, 1, writebin) != 1) {
							printf("write record1 error: %s\n",dest->tableFileName);
							return 0;
						}
                    				if (fwrite(record2, table2->reclen, 1, writebin) != 1) {
							printf("write record2 error: %s\n",dest->tableFileName);
							return 0;
						}
                 			}
            			}
          		}
       		}
   	}
   fclose(writebin);
   //free(record1);
   //free(record2);
   return 1;
}
   
//create a new table with combined information of two tables
bool combinTable(struct _table *table1, struct _table *table2, char* column1, char *column2, struct _table *dest, char *a) {
	//load table1 and table2; get required index for data comparation
   int index1=loadTableForCombine(table1, column1);
   int index2=loadTableForCombine(table2, column2);
   //table1&2 is already combined table
   //create schemafile and bin file for dest & combine two schema file
   if(!combineSchema(table1, table2, dest,a)) {
   		printf("combineSchema fail\n");
      return 0;
   }
//check for load dest->schema  
   if(!loadSchema(dest)){
   		printf("%s does not exist. in combineTable function\n", dest->schemaName);
   return 0;
   }
	writeNewTableBinFile(table1, table2, dest, index1, index2);
   return 1;   
}

void getNewLine(char *buffer,char *status) {
	status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	printf("===> %s", buffer);
   return;
}

bool assignTableName(char *token,struct _table *table) {
	table->tableFileName = calloc(1, strlen(token) + 4 + 1);
	table->schemaName = calloc(1, strlen(token) + 7 + 1);
	strcpy(table->tableFileName, token);
	strcat(table->tableFileName, ".bin");
	strcpy(table->schemaName, token);
	strcat(table->schemaName, ".schema");
   return 1;
}

bool createIndex(struct _table *fromTable,struct _table *table,char *buffer,char *field, char *tempTableName) {
   char *createCmd=malloc(MAXINPUTLENGTH);
   char *addCmd=malloc(MAXINPUTLENGTH);
   char *sortmeFile="sortme.sortme";
   char *tempFile="temp";
   char *temp=malloc(MAXINPUTLENGTH);
   char *token=malloc(MAXINPUTLENGTH);
   
   FILE *createSortme=fopen(sortmeFile, "w");
   FILE *createTemp=fopen(tempFile, "w");
   fclose(createSortme);
   fclose(createTemp);
   
   int SizeofFile=getFileSize(fromTable);
   loadSchema(fromTable);
   char *record = calloc(1, fromTable->reclen);
   
   sprintf(createCmd, "CREATE TABLE %s\n", tempTableName);
   
   FILE *readSchema=fopen(fromTable->schemaName, "r");
   FILE *createSchema=fopen(table->schemaName, "w");
   fclose(createSchema);
   FILE *createBin=fopen(table->tableFileName, "w");
   fclose(createBin);
   FILE *writeSchema=fopen(table->schemaName, "a");
   
   while (1) {
		if (fgets(temp, 998, readSchema) == NULL) {  //get every line of stdin
			break;
		}
		if (temp[0] == '\n') {
			break;
		}
		char *temp1=malloc(MAXINPUTLENGTH);
      char *tempField=malloc(MAXINPUTLENGTH);
      char *tokenField=malloc(MAXINPUTLENGTH);
      strcpy(temp1,temp);
      strcpy(tempField,field);
     token = strtok(temp1, " ");
     if (strncmp(token,"ADD",3)==0) {
        token = strtok(NULL, " ");
			if(strchr(tempField,',')){
				tokenField=strtok(tempField,",");}
			else{
				tokenField = strtok(tempField, " ");}
        while (strncmp(tokenField, ";", 1) != 0) {
           if(tokenField[sizeof(tokenField)-1]==',') {
              char *temp00=malloc(MAXINPUTLENGTH);
              strncpy(temp00,tokenField,sizeof(tokenField)-1);
              strcpy(tokenField,temp00);
           }
//printf("tokenField: %s.....token: %s\n",tokenField,token);
           if(strncmp(tokenField, token, strlen(token))==0) {
              fwrite(temp,strlen(temp),1,writeSchema);
           }
				tokenField= strtok(NULL, " ");
        }
     }
   }
   fclose(writeSchema);
   fclose(readSchema);
//ipeople.schema is ready
   loadSchema(table);
   FILE *sortme=fopen(sortmeFile, "a");
   int index=0;
   for (int z = 0; z<SizeofFile / fromTable->reclen; ++z) {
		if (getRecord(z, record, fromTable)) {
			for(int q=0;q<table->fieldcount;++q) {
				for(int w=0;w<fromTable->fieldcount;++w) {
					if(strncmp(table->fields[q].fieldName,fromTable->fields[w].fieldName,strlen(fromTable->fields[w].fieldName))==0) {
                for(int p=0;p<fromTable->fields[w].fieldLength;++p) {
                   if(record[index+p]=='\0' ||record[index+p]=='\n')
                      record[index+p]==' ';
                }
						fprintf(sortme,"%s",&record[index]);
                if(q+1<table->fieldcount)
                   fprintf(sortme,",");
                break;
              }
					index=index+fromTable->fields[w].fieldLength;
           }
           index=0;
			}
        fprintf(sortme,"\n");
                   
		}
	} 
   fclose(sortme);
   system("sort <sortme.sortme >temp");
//sorted file
   char *insertCmd=malloc(MAXINPUTLENGTH);
   FILE *readtemp=fopen(tempFile, "r");
   temp=malloc(MAXINPUTLENGTH);
   while(fgets(temp, 998, readtemp) != NULL) {
		if (temp[0] == '\n') {
			break;
		}
//printf("temp: %s\n",temp);
		sprintf(insertCmd, "INSERT INTO %s %s", tempTableName,temp);
//printf("insertCmd: %s\n",insertCmd);
      processCommand(insertCmd);
   }
   fclose(readtemp);
}

bool readIndexCreate(struct _table *table,char *buffer,char *field) {
	char *status=malloc(sizeof(char));
	char *token=malloc(MAXINPUTLENGTH);
  	char *tempTableName=malloc(MAXINPUTLENGTH);
   	struct _table fromTable;
   	struct _multTable multTable;//if from mult table. use it
   	struct _whereField whereField; //assign for WHEREandAND, no use
   	int sign[10]={0};//assign for WHEREandAND, no use
	token = strtok(buffer, " ");
	strcpy(tempTableName,token);
    	assignTableName(token,table);
    	token = strtok(NULL, " ");
	if(strncmp(token, "USING", 5) == 0) { //collect field that need to be in the index
		token = strtok(NULL, "\n");
//printf("token: %s\n", token);
		strcpy(field, token);
		strcat(field, " ;");
//printf("field: %s\n",field);
	}
	getNewLine(buffer, status);
	token = strtok(buffer, " ");
   	if(strncmp(token, "FROM", 4) == 0) {
      	token = strtok(NULL, "\n");
      	fromMultTable(token,&multTable);
      	if (strcmp(multTable.tables[1].tableFileName,"0")==0)//if there is only one table,no combination
      		fromTable=multTable.tables[0];
      	getNewLine(buffer, status);
      	if(strncmp(buffer, "WHERE", 5)==0)
		WHEREandAND(table,buffer,status,&whereField,&multTable,&fromTable, sign);
      
   	}
   	createIndex(&fromTable,table,buffer,field, tempTableName);
}

bool createCommand(struct _table *table,char buffer[MAXINPUTLENGTH]) {
   char *token=malloc(MAXINPUTLENGTH);
   token = strtok(buffer, " ");
   token = strtok(NULL, " ");
	if (strncmp(token, "TABLE", 5) == 0) {
		token = strtok(NULL, " ");
		assignTableName(token,table);
		CreateTable(table);
		return 1;
	}
   else if(strncmp(token, "INDEX", 5) == 0) {
		char *field=malloc(MAXINPUTLENGTH);
		token = strtok(NULL, "\n");
		readIndexCreate(table,token,field);
		
   }
   else{
      printf("Error in createCommand\n");
      return 0;
   }
   //free(token);
   return 1;
}
bool insertCommend(struct _table *table,char buffer[MAXINPUTLENGTH]) {
   char *token=malloc(MAXINPUTLENGTH);
   token = strtok(buffer, " ");
   token = strtok(NULL, " ");
	if (strncmp(token, "INTO", 4) == 0) {
		token = strtok(NULL, " ");
		assignTableName(token,table);
		token=malloc(MAXINPUTLENGTH);
		token=strtok(NULL, "\n");
		loadSchema(table);
		int k = loadDatabase(table,token);
	}
   else{
      printf("Error in insertCommand\n");
      return 0;
   }
   return 1;
}
bool dropCommend(struct _table *table,char buffer[MAXINPUTLENGTH]) {
   char *token=malloc(MAXINPUTLENGTH);
   token = strtok(buffer, " ");
   token = strtok(NULL, " ");
   if (strncmp(token, "TABLE", 5) == 0) {
		token = strtok(NULL, "\n");
        assignTableName(token,table);
        if(remove(table->tableFileName)!=0)
           printf("remove bin file unsuccessfully\n");
        if(remove(table->schemaName)!=0)
           printf("remove schema file unsuccessfully\n");
   }
   else{
      printf("Error in dropCommand\n");
      return 0;
   }
   //free(token);
   return 1;
}
void collectSelectedField(char buffer[MAXINPUTLENGTH],char *field) {
   char *token=malloc(MAXINPUTLENGTH);
   token = strtok(buffer, " ");
   token = strtok(NULL, " ");
	while (strncmp(token, "\n", 1) != 0) {
		if (token[strlen(token) - 1] == ',')
			token[strlen(token) - 1] = ' ';
		else {
			strcat(field, token);
			break;
		}
           
		strcat(field, token);
		token = strtok(NULL, " ");
		}
	strcat(field, " ;");
   return;
}

void fromMultTable(char *token,struct _multTable *multTable) {
   char *rest=malloc(MAXINPUTLENGTH);
	int z=0;
	while(rest=strchr(token,',')) {
		char *token1=calloc(1, strlen(rest));
		token1= strtok(token, ", ");
		if(token1[0]==' ') {
			assignTableName(&token1[1],&multTable->tables[z]);
        }
		else{
			assignTableName(token1,&multTable->tables[z]);
        }
		++z;
		token=strtok(NULL, "\n");
		trimwhitespace(token);
	}
	if(token[0]==' ') {
		assignTableName(&token[1],&multTable->tables[z]);
	}
	else{
 		assignTableName(token,&multTable->tables[z]);
	}
	++z;
//end of appending FROM table into array
	multTable->tables[z].tableFileName = calloc(1, sizeof(char));
	multTable->tables[z].schemaName = calloc(1, sizeof(char));
	strcpy(multTable->tables[z].tableFileName, "0");
	strcpy(multTable->tables[z].schemaName, "0");
}

bool collectFromTable(struct _table *table,char buffer[MAXINPUTLENGTH],struct _multTable *multTable, char *status) {
   char *token=malloc(MAXINPUTLENGTH);
   if (status!=NULL) {
		token = strtok(buffer, " ");          
		if (strncmp(token, "FROM", 4) == 0) {
			token = strtok(NULL, "\n");
			fromMultTable(token,multTable);
		}
		
	}
   else{
      printf("Error in collectFromTable: no new line\n");
      return 0;
   }
   //free(token);
   return 1;
}
           
bool WHEREandAND(struct _table *table,char *buffer,char *status,struct _whereField *whereField,struct _multTable *multTable,struct _table *dest, int sign[]){
   	char *token=malloc(MAXINPUTLENGTH);
   	char *tempbuffer=malloc(MAXLENOFFIELDNAMES);
   	char *column1=malloc(MAXLENOFFIELDNAMES);
	char *column2=malloc(MAXLENOFFIELDNAMES);
   	int j=0;
   	int n=1;
   	strcpy(tempbuffer,buffer);
   	while (strncmp(buffer,"END",3)!=0) {
		token = strtok(buffer, " ");  
		if ((strncmp(token, "AND", 3)==0)||(strncmp(token, "WHERE", 5) == 0)) {
			if(strchr(tempbuffer,'"')){ //column = "something"
				token = strtok(NULL, " ");
				strcpy(whereField->fields[j].fieldName, token);
				token =strtok(NULL, " \"");
           			if(strcmp(token, ">=")==0)
              				sign[j]=1;
           			else if(strcmp(token, "<=")==0)
              				sign[j]=-1;
				token =strtok(NULL, "\"\n");
				strcpy(whereField->fields[j].fieldType, token);
				j+=1;
				getNewLine(buffer, status);
				strcpy(tempbuffer,buffer);
			}
			else{ //column1=column2
				token=strtok(NULL, " ");
				strcpy(column1,token);
				token=strtok(NULL, " ");
				token=strtok(NULL, "\n");
				strcpy(column2,token);
				if (strcmp(multTable->tables[n].tableFileName,"0")!=0) {
					char a = n+'0';
//printf("Enter strcmp\na: %c\n",a);
					combinTable(&multTable->tables[n-1], &multTable->tables[n], column1, column2,dest, &a);
					multTable->tables[n]=*dest;
					n++;
//printf("n after ++: %d\n",n); 
				}
				else {
					printf("table combine out of range\n");
				}
				getNewLine(buffer, status);
				strcpy(tempbuffer,buffer);
                     
			}
		}
		else {printf("input line does not start with WHERE or AND in WHEREandAND function.\n"); return 0;}
   }
   if (strcmp(multTable->tables[n].tableFileName,"0")==0)//if there is only one table,no combination
      dest=&multTable->tables[0];
   //combined all table already and whereField is ready
	strcpy(whereField->fields[j].fieldName,"0");
	strcpy(whereField->fields[j].fieldType,"0");
   //free(tempbuffer);
   //free(column1);
	//free(column2);*/
   return 1;
}

bool showSelectResult(struct _table *table,char *buffer,char *field,struct _whereField *whereField,struct _multTable *multTable,struct _table *dest, int sign[]){
   int SizeofFile=getFileSize(dest);
   loadSchema(dest);
   char *record = calloc(1, dest->reclen);
//print out trace statement while search index table  
   /*if(multTable->tables[0].tableFileName[0]=='i' && (strncmp(whereField->fields[0].fieldName,"0",1)!=0 )) {
      int num=0;
      int base=0;
		printf("TRACE: ");
		int recNum=SizeofFile/dest->reclen;
      if(recNum%2==1)
         num=recNum/2+1;
      else
         num=recNum/2;
      FILE *readTemp=fopen("temp", "r");
      for(int g=0;g<num;++g)
         fgets(record,998, readTemp);
      printf("%s",record);
      rewind(readTemp);
      int k=0;
      while(strncmp(record,whereField->fields[0].fieldType,3)!=0) {
         //printf("num: %d\n",num);
         if(k>3)
            break;
         printf("TRACE: ");
         if((whereField->fields[0].fieldType[0])<record[0]) {
				recNum=num;
            if((base+num)%2==1)
         			num=(base+num)/2;
      			else
         			num=(base+num)/2;
         }
         else{
            base=num;
         	if((num+recNum)%2==1)
         		num=(num+recNum)/2+1;
      			else
         		num=(num+recNum)/2;
         }
         for(int n=0;n<num;++n)
         	fgets(record,998, readTemp);
      		printf("%s",record);
         rewind(readTemp);
         ++k;
      }
      fclose(readTemp);
   }*/
   
   for (int z = 0; z<SizeofFile / dest->reclen; ++z) {
		if (getRecord(z, record, dest)) {
			showRecord(dest->fields, record, dest->fieldcount, field, whereField, sign);
		}
	} 
   //free(record);
	return 1;
}

bool selectCommend(struct _table *table,char buffer[MAXINPUTLENGTH]) {
	char *status=malloc(sizeof(char));
	struct _multTable multTable;
	struct _table dest; 
	struct _whereField whereField; 
   int sign[10]={0};//***'=' is 0***'>=' is 1; '<=' is -1;
	char *field = calloc(100, sizeof(char));
	collectSelectedField(buffer,field);
	getNewLine(buffer, status);
   strcpy(buffer, buffer);
	collectFromTable(table,buffer,&multTable, status);
	if (strcmp(multTable.tables[1].tableFileName,"0")==0)//if there is only one table,no combination
      dest=multTable.tables[0];
	getNewLine(buffer, status);
	WHEREandAND(table,buffer,field,&whereField,&multTable,&dest, sign);
	showSelectResult(table,buffer,field,&whereField,&multTable,&dest,sign);
   //free(status);
   //free(field);
   return 1;
} 
      
   
void processCommand(char buffer[MAXINPUTLENGTH]) {
	struct _table table;
	if (strncmp(buffer, "CREATE", 6) == 0) {
		createCommand(&table,buffer);
		return;
	}
	if (strncmp(buffer, "INSERT", 5) == 0) { //start with load
		insertCommend(&table,buffer);
		return;
	}
   	if (strncmp(buffer, "DROP", 4) == 0) {  //drop table
		dropCommend(&table,buffer);
		return;
   }
	if (strncmp(buffer, "SELECT", 6) == 0) {
		selectCommend(&table,buffer);
		return;
	}
}






int main() {
	static char buffer[MAXINPUTLENGTH];
	memset(buffer, 0, MAXINPUTLENGTH);
	printf("Welcome!\n");
	char *status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	while (status != NULL) {
		trimwhitespace(buffer);
		if (strlen(buffer) < 5 )
			break; // not a real command, CR/LF, extra line, etc.
		printf("===> %s\n", buffer);
		processCommand(buffer);
		status = fgets(buffer, MAXINPUTLENGTH - 1, stdin);
	}
	printf("Goodbye!\n");
	return 0;
}


