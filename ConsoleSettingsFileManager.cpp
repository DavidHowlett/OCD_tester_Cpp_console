// Standard Header Files ---------------------------------------------------
#include <stdio.h>
#include <windows.h>
// My Header Files ---------------------------------------------------------
#include "ConsoleSettingsFileManager.h"
//---------------------------------------------------------------------------
SettingsFileManager::SettingsFileManager(){
}
SettingsFileManager::~SettingsFileManager(){
}
SettingsFileManager::ReadFile(){
	int BrokenSettingsTest = 0;
	FilePointer = fopen ("settings file.txt","r");
	if(NULL==FilePointer){
		printf("Could not open settings file\n");
		printf("You should make sure that the settings file exists, is in the same folder as the .exe and is formatted correctly\n");
		Sleep(9999999);
		return(1);
	}else{
		fscanf (FilePointer,"%d",&LatestGoodFlowmeterPort);			fgets (SettingsDescriptors[0],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%d",&DataGatheringCycleTime);      fgets (SettingsDescriptors[1],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%d",&LastRawDataFileNumber);       fgets (SettingsDescriptors[2],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%d",&LastProcessedDataFileNumber);	fgets (SettingsDescriptors[3],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%f",&TriggerFlow);									fgets (SettingsDescriptors[4],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%f",&AntibounceWaitTime);					fgets (SettingsDescriptors[5],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%f",&TimeToBeIncludedInPulseOnEitherSideOfTriggerTime);fgets (SettingsDescriptors[6],MaxCharsInSettingDescriptor,FilePointer);
		fscanf (FilePointer,"%d",&BrokenSettingsTest);					fgets (SettingsDescriptors[7],MaxCharsInSettingDescriptor,FilePointer);

		fclose (FilePointer);
		if (1234567890==BrokenSettingsTest){
			printf("Settings file read successfully\n");
			return(0);
		}else{
			printf("Did not read settings file correctly\n");
			printf("You should make sure that the settings file exists, is in the same folder as the .exe and is formatted correctly\n");
			Sleep(9999999);
			return(1);
		}
	}
}
SettingsFileManager::WriteFile(){
	FilePointer = fopen ("settings file.txt","w");
	if(NULL==FilePointer){
		printf("Could not open settings file\n");
		return(1);
	}
	int BrokenSettingsTest = 1234567890;
	fprintf (FilePointer,"%d%s",LatestGoodFlowmeterPort,SettingsDescriptors[0]);
	fprintf (FilePointer,"%d%s",DataGatheringCycleTime,SettingsDescriptors[1]);
	fprintf (FilePointer,"%d%s",LastRawDataFileNumber,SettingsDescriptors[2]);
	fprintf (FilePointer,"%d%s",LastProcessedDataFileNumber,SettingsDescriptors[3]);
	fprintf (FilePointer,"%f%s",TriggerFlow,SettingsDescriptors[4]);
	fprintf (FilePointer,"%f%s",AntibounceWaitTime,SettingsDescriptors[5]);
	fprintf (FilePointer,"%f%s",TimeToBeIncludedInPulseOnEitherSideOfTriggerTime,SettingsDescriptors[6]);
	fprintf (FilePointer,"%d%s",BrokenSettingsTest,SettingsDescriptors[7]);

	fclose (FilePointer);
	return(0);
}

