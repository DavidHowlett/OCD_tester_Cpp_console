#include "Functions.h"
//#include <Math.h>
//#include <stdlib.h>
//#include <string.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

char Ux39UjK[]={13,0};
AnsiString CRString=Ux39UjK;

//******************************************************************************
int FindCharInBuffer(char *buffer, char SoughtCharacter)
//Seek "SoughtCharacter" in buffer. Return -1 for not found, or position.
//******************************************************************************
{
	for (unsigned int i=0; i < strlen(buffer); i++) {
		if (buffer[i]==SoughtCharacter) {
			return i;
		}
	}
	return -1; //Not found, despite reaching the end of the buffer
}

//Identify where to burn a new hole with the laser.
//******************************************************************************
char* uCase(char *buffer)
//return uppercase(buffer): Silly default command set doesn't seem to support this.
//******************************************************************************
{
	char* str;
	unsigned int lenstr;
	lenstr=strlen(buffer);
	str=(char*)malloc(lenstr+1);
	strcpy(str,buffer);
	for (unsigned int i=0; i < lenstr; i++) {
		if ((str[i]>='a') &&(str[i]<='z')) {
			str[i]=str[i]+('A'-'a');
		}
	}
	return str;
}

