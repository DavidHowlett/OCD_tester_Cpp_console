// Overview of program (documentation for programers) ----------------------
/*

There is a device designed by "C2M design" called the OCD (oxygen conserving device).
The principle is to give a person who is on oxygen a single pulse of oxygen at the beginning of their breath because oxygen given at other times is wasted.
To test this device a flow meter is connected to the output of the OCD and the flow is monitored.
The OCD can be rigged to produce regular pulses.
The raw data produced by the monitoring and the processed data are both written to test files.
The purpose of this program is to monitor and analyse those pulses to determine if the OCD is working correctly.
This program will support both Alicat and TSI mass flow meters by the time it is finished.
The first iteration of this program was built using a command line interface.

This program was written by David Howlett in 2012 while working at C2M design.
C2M design should be contacted at: andrew.tatarek@c2mdesign.co.uk
David can be contacted at: david.howlett@c2mdesign.co.uk (work) or david@howletts.org.uk (personal)

Overview of each class:
	- SettingsFileManager
		There are various pieces of data that need to be preserved from one time the program runs to the next.
		These are all stored in the Settings file.
		This class reads and writes the Settings file.
		The data and the comments in the Settings file are user modifiable when the program is not running
		When Settings are needed they can be requested of this class.
	- FlowmeterManager
		This class acts as a layer of abstraction.
		The main program asks this class for mass flow data and this class then asks the flowmeters.
		This means that a variety of flow meters can be used and the main program does not need to know where the data comes from.
	- TsiFlowmeter
		This class communicates with a TSI mass flow meter.
		When the mass flow is required this class can be asked.
		Temperature and pressure can also be asked from the TSI flow meter but this has not been implemented in this class yet.
	- AlicatFlowmeter
		This class was originally written by Alan Howlett at Data Technologies LTD.
		As a result the coding conventions in this class are different from the rest of the code.
		The class is put in a separate folder with all the other files it needs to function.

*/
// Standard Header Files ---------------------------------------------------
#include <stdio.h>
#include <iostream.h>
#include <time.h> // this is not used to give the time but it is used to process the time provided by the API
#include <tchar.h> // this is only used for t_main
#include <windows.h>
#include <cassert>
// My Header Files ---------------------------------------------------------
#include "ConsoleSettingsFileManager.h"
#include "ConsoleFlowmeterManager.h"
// Globals -----------------------------------------------------------------
const int 	MaxPulsesInAReading = 100;
const int 	MaxReadingsBeforeOutputtingAnalysisFile = 100;
const int 	RawDataArraySize = 100000;
SettingsFileManager Settings;
FlowmeterManager GenericFlowmeter;  // this creates a class of type: flowmeter_selector
bool 	FlowmeterExists = false;
int    	ReadingsStoredInArrays = 0;      // I choose to define a reading to be a set of pulses that are grouped together and avaraged to give a result
double 	TimeOfReading			  	[RawDataArraySize]; // this is the time in milliseconds
float 	FlowReading				[RawDataArraySize]; // the raw data arrays have been declared here to avoid putting the data on the stack
int		RawDataFileAssosiatedWithReading [MaxReadingsBeforeOutputtingAnalysisFile];
int 	PulsesInReading			[MaxReadingsBeforeOutputtingAnalysisFile];// each reading will be made up of many pulses (3 is expected)
bool  	TooManyPulsesInReading		[MaxReadingsBeforeOutputtingAnalysisFile];// this notes if there was more data then was recorded
float	PulsePeakFlow 			[MaxPulsesInAReading] [MaxReadingsBeforeOutputtingAnalysisFile];
float	PulseCycleTime 			[MaxPulsesInAReading] [MaxReadingsBeforeOutputtingAnalysisFile];
float	PulseOnTime 				[MaxPulsesInAReading] [MaxReadingsBeforeOutputtingAnalysisFile];
float	PulseVolume 				[MaxPulsesInAReading] [MaxReadingsBeforeOutputtingAnalysisFile];
float	AveragePeakFlow 			[MaxReadingsBeforeOutputtingAnalysisFile];
float	AverageCycleTime 			[MaxReadingsBeforeOutputtingAnalysisFile];
float	AveragePulseOnTime 		[MaxReadingsBeforeOutputtingAnalysisFile];
float	AveragePulseVolume 		[MaxReadingsBeforeOutputtingAnalysisFile];
// Function Declarations----------------------------------------------------
void	Setup();
void	ZeroRawDataArrays();
void	ZeroProcessedDataArrays();
void	UserInteractionAndAction(); // gets user input and performs the action requested by the user
void	GetData(int number_of_pulses_to_record);      // this records data from the flowmeter for a user specified number of seconds, calling it will create one reading
int		ProcessData();
void	FindPulseAverages(int reading_of_interest);
void	PutProcessedDataOnConsole();
void	SaveRawData();     // this saves the raw experemental data to a text file. the int passed to it indicates the file number that it should be saved to
void	SaveProcessedData();

//--------------------------------------------------------------------------
#pragma argsused
int _tmain(int argc, _TCHAR* argv[]){
	Setup();
	UserInteractionAndAction();
	return 0;
}
void Setup(){
	ZeroRawDataArrays();
	ZeroProcessedDataArrays();
	Settings.ReadFile();
	GenericFlowmeter.Setup(&Settings);
}
void ZeroRawDataArrays(){
	for (int i = 0; i < RawDataArraySize; i++) {
		TimeOfReading[i]=0;
		FlowReading[i]=0;
	}
}
void ZeroProcessedDataArrays(){
	for(int i = 0; i < MaxReadingsBeforeOutputtingAnalysisFile; i++) {
		AveragePeakFlow[i] = 0;
		AverageCycleTime[i] = 0;
		AveragePulseOnTime[i] = 0;
		AveragePulseVolume[i] = 0;
		PulsesInReading[i] = 0;
		TooManyPulsesInReading	 [i] = false;
		for (int j = 0; j < MaxPulsesInAReading; j++) {
			PulsePeakFlow[j][i]=0;
			PulseCycleTime[j][i]=0;
			PulseOnTime[j][i]=0;
			PulseVolume[j][i]=0;
		}
	}
}
void UserInteractionAndAction(){
	char UserCommand[100]; // used to record user input
	int UserNumber = 0;
	bool ShouldContinue = true;
	while(ShouldContinue){
		printf("Enter user input\n");
		scanf("%s",&UserCommand);
		if('e'==UserCommand[0]){
			ShouldContinue = false;
		}
		else if('r'==UserCommand[0]){
			scanf("%d",&UserNumber); // the user number is the number of pulses that the program should record in the current reading
			if (GenericFlowmeter.FlowmeterReady) {
				if (UserNumber <= MaxPulsesInAReading){
					ZeroRawDataArrays();
					GetData(UserNumber);
					PutProcessedDataOnConsole();
					SaveRawData();
				}else{
					printf("you cannot have more then %d pulses in a reading\n",MaxPulsesInAReading);
				}
			}else{
				printf("You can't do this operation because the flowmeter is not Setup yet\n");
			}
			if(ReadingsStoredInArrays>=MaxReadingsBeforeOutputtingAnalysisFile){// this is included to handle the overflow caused by the user taking too many readings before telling the system to output an analyis file
				SaveProcessedData();
			}
		}
		else if('o'==UserCommand[0]){
			SaveProcessedData();
		}
		else{
			printf("Error - user input not recognised\n");
		}
	}
}
void GetData(int pulses_to_record){
	int RecordedPulses = 0;
	while(-2==GenericFlowmeter.MassFlow()){
		Sleep(1);// the flowmeter returns -2 when it does not have data, this code waits until the flowmeter has data, idealy this code should never need to run as the flowmeter should have data by this point in the program. If the flowmeter is connected backwards it will give negetive flow values but this will not cause  problems unless it consistantly returns a value of -2
	}
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER start_time, current_time;           // ticks
	QueryPerformanceFrequency(&frequency);  // get ticks per second
	QueryPerformanceCounter(&start_time);     // record the time that the GetData function started
	//int pulse_starts = 0;
	for(int i=0;i<RawDataArraySize;i++){     // this stops the test when all the pulses needed have bee recorded or the array runs out of space
		QueryPerformanceCounter(&current_time);
		TimeOfReading[i] = double(current_time.QuadPart - start_time.QuadPart)/double(frequency.QuadPart); // this calculates the time in seconds from the initiation of this function
		FlowReading[i] = GenericFlowmeter.MassFlow();
		printf("%f %f\n",TimeOfReading[i],FlowReading[i]);
		RecordedPulses = ProcessData();
		if (RecordedPulses >= pulses_to_record)
			break;
		Sleep(Settings.DataGatheringCycleTime);
	}
	assert(RecordedPulses == pulses_to_record);
	PulsesInReading[ReadingsStoredInArrays] = RecordedPulses;
	FindPulseAverages(ReadingsStoredInArrays);
	ReadingsStoredInArrays++;    // this records that a reading has happened
}
int ProcessData(){ // I need to find the pulse duration, the cycle time and the volume in this function
	double IntegratedVolume = 0; // this variable records the volume that passed through the OCD in the part of the pulse processed so far
	float CurrentBiggestFlow = -1000;
	int MostRecentUp = -1000;
	int RecordedPulses = 0;
	bool Recording = false;
	for (int i=2;(i<(RawDataArraySize-1)) && TimeOfReading[i] > 0;i++){// the unusual fence post conditions are to stop the integration line reading outside the area that it should
		assert(TimeOfReading[i] > 0);
		if (FlowReading[i] > CurrentBiggestFlow){
			CurrentBiggestFlow = FlowReading[i];
		}
		// The below logic does the integration.
		// The "1000/60" converts from litres per minute to millilitres per second
		// The "(TimeOfReading[i+1]-TimeOfReading[i-1])/2" gives the best estimate that I know of for the length of time associated with a reading
		// For easy understanding I want to write the algebraically correct expression:
		// IntegratedVolume = IntegratedVolume + ((TimeOfReading[i+1]-TimeOfReading[i-1])/2)*FlowReading[j]*1000/60;
		// To make the above line actually function in code and provide and accurate answer I rearrange it to be the algebraically equivalent:
		// IntegratedVolume = IntegratedVolume + (TimeOfReading[i+1]-TimeOfReading[i-1])*FlowReading[j]*1000/60/2;
		// and then:
		IntegratedVolume = IntegratedVolume + (TimeOfReading[i]-TimeOfReading[i-2])*FlowReading[i-1]*(double)25/(double)3;
		if (FlowReading[i] >= Settings.TriggerFlow && FlowReading[i-1] < Settings.TriggerFlow){
			if (Recording){
				PulseVolume[RecordedPulses] [ReadingsStoredInArrays]=IntegratedVolume;
				PulseCycleTime [RecordedPulses] [ReadingsStoredInArrays] =  TimeOfReading[i] - TimeOfReading[MostRecentUp];
				PulsePeakFlow [RecordedPulses] [ReadingsStoredInArrays] = CurrentBiggestFlow;
				CurrentBiggestFlow = -1000;
				RecordedPulses++;
			}
			IntegratedVolume = 0;
			MostRecentUp = i;
			Recording = true;
		}
		if ((FlowReading[i] <= Settings.TriggerFlow) && (FlowReading[i-1] > Settings.TriggerFlow) && Recording){
			PulseOnTime [RecordedPulses] [ReadingsStoredInArrays] = TimeOfReading[i] - TimeOfReading[MostRecentUp];
		}
	}
	PulsesInReading [ReadingsStoredInArrays] = RecordedPulses;
	return(RecordedPulses);
}
void FindPulseAverages(int reading_of_intrest){// this take the avarage of the quantitiys derived though analysis over all the pulses in the reading passed to it
	AveragePeakFlow[reading_of_intrest]=0;
	AverageCycleTime[reading_of_intrest]=0;
	AveragePulseOnTime[reading_of_intrest]=0;
	AveragePulseVolume[reading_of_intrest]=0;
	for(int i=0 ; i<PulsesInReading[reading_of_intrest];i++){
		AveragePeakFlow[reading_of_intrest]=AveragePeakFlow[reading_of_intrest]+PulsePeakFlow [i][reading_of_intrest];
		AverageCycleTime[reading_of_intrest]=AverageCycleTime[reading_of_intrest]+PulseCycleTime [i][reading_of_intrest];
		AveragePulseOnTime[reading_of_intrest]=AveragePulseOnTime[reading_of_intrest]+PulseOnTime [i][reading_of_intrest];
		AveragePulseVolume[reading_of_intrest]=AveragePulseVolume[reading_of_intrest]+PulseVolume [i][reading_of_intrest];
	}
	if(PulsesInReading[reading_of_intrest] != 0){
		AveragePeakFlow[reading_of_intrest]=AveragePeakFlow[reading_of_intrest]/PulsesInReading[reading_of_intrest];
		AverageCycleTime[reading_of_intrest]=AverageCycleTime[reading_of_intrest]/PulsesInReading[reading_of_intrest];
		AveragePulseOnTime[reading_of_intrest]=AveragePulseOnTime[reading_of_intrest]/PulsesInReading[reading_of_intrest];
		AveragePulseVolume[reading_of_intrest]=AveragePulseVolume[reading_of_intrest]/PulsesInReading[reading_of_intrest];
	}
}
void PutProcessedDataOnConsole(){
	if (0 != PulsesInReading[ReadingsStoredInArrays-1]){
		printf("The pulse peak flow, duration, cycle time and volume are:\n");
		for(int i = 0 ;i<PulsesInReading[ReadingsStoredInArrays-1];i++){
			printf("%f\t%f\t%f\t%f\n",PulsePeakFlow[i][ReadingsStoredInArrays-1],PulseOnTime[i][ReadingsStoredInArrays-1],PulseCycleTime[i][ReadingsStoredInArrays-1],PulseVolume[i][ReadingsStoredInArrays-1]);
		}
		printf("The average pulse peak flow, duration, cycle time and volume are:\n");
		printf("%f\t%f\t%f\t%f\n",AveragePeakFlow[ReadingsStoredInArrays-1],AveragePulseOnTime[ReadingsStoredInArrays-1],AverageCycleTime[ReadingsStoredInArrays-1],AveragePulseVolume[ReadingsStoredInArrays-1]);
	}else{
		printf("No pulses available for analysis\n");
	}
	if(TooManyPulsesInReading	[ReadingsStoredInArrays-1]){
		printf("There were too many pulses in this reading meaning that\nsome of them were not recorded properly and are are not displayed above\n");
	}
}
void SaveRawData(){ // this function needs to know the current reading number and it needs to know the most recently written raw Settings file number
	char FileName[50];
	sprintf(FileName,"raw data %d.txt",Settings.LastRawDataFileNumber+1);
	Settings.WriteFile(); // this record the change in the most_recently_used_raw_data_file_number
	RawDataFileAssosiatedWithReading [ReadingsStoredInArrays-1]=Settings.LastRawDataFileNumber+1;
	FILE * pFile;
	pFile = fopen (FileName,"w+");
	if( pFile==NULL)
		printf("Error: could not access the test data file\n");
	else{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer [80];
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		fprintf(pFile,"The local date and time when the below data was recorded to file:\n");
		strftime (buffer,80,"Date: %Y-%m-%d Time: %H:%M:%S",timeinfo);
		fprintf(pFile,"%s\n",buffer);
		fprintf(pFile,"The left column shows the number of seconds since data logging began and the right column shows the mass flow reading at that time\n\n");
		for (int i = 0; i < RawDataArraySize; i++) {
			if(i>5 && TimeOfReading[i]<0.01) // the array is intialised to contain only zeros exept in the region in which there is data. it is expected that once this loop is 5 readings into the data the time will no longer be 0
				break;                                // this means that the text file is truncated to the size of the actual data
			fprintf(pFile,"%f\t%f\n",TimeOfReading[i],FlowReading[i]);
		}
		printf("Raw data file %d created\n",Settings.LastRawDataFileNumber+1);
		Settings.LastRawDataFileNumber++;
		Settings.WriteFile();
	}
	fclose (pFile);
}
void SaveProcessedData(){
	FILE * pFile;
	char FileName[50];
	sprintf(FileName,"processed data %d.txt",(Settings.LastProcessedDataFileNumber+1)); // the idea is that this causes the analysed data file numbers to be numbered sequentialy
	pFile = fopen (FileName,"w");
	if(NULL==pFile){
		printf("Could not open processed data file\n");
	}else{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer [80];
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		fprintf(pFile,"The local date and time when the below data was recorded to file:\n");
		strftime (buffer,80,"Date: %Y-%m-%d Time: %H:%M:%S",timeinfo);
		fprintf(pFile,"%s\n\n",buffer);

		if(ReadingsStoredInArrays>0){
			for (int i = 0; i < ReadingsStoredInArrays; i++) {
				fprintf(pFile,"Below is reading %d assosiated with raw data file %d\n",(i+1),RawDataFileAssosiatedWithReading [i]);
				if(PulsesInReading[i]>0){
					fprintf(pFile,"The pulse peak flow, duration, cycle time and volume are:\n");
					for(int j=0; j < PulsesInReading[i] ; j++){
						fprintf(pFile,"%f\t%f\t%f\t%f\n",PulsePeakFlow[j][i],PulseOnTime[j][i],PulseCycleTime[j][i],PulseVolume[j][i]);
					}
					fprintf(pFile,"The average pulse peak flow, duration, cycle time and volume are:\n");
					fprintf(pFile,"%f\t%f\t%f\t%f\n",AveragePeakFlow[i],AveragePulseOnTime[i],AverageCycleTime[i],AveragePulseVolume[i]);
					if(TooManyPulsesInReading	[i]){
						fprintf(pFile,"There were too many pulses in this reading meaning that\nsome of them were not recorded properly and are are not displayed above\n");
					}
				}else{
					fprintf(pFile,"There are no pulses available for analysis in this reading\n");
				}
				fprintf(pFile,"\n"); // this aids readablitiy for humans by visualy seperateing each reading
			}
		}else{
			fprintf(pFile,"There were no readings\n");
		}
		Settings.LastProcessedDataFileNumber++;
		Settings.WriteFile(); // this records that the most_recently_used_analysed_data_file_number has increaced in value
		ReadingsStoredInArrays = 0;
		printf("Processed data file %d created\n",Settings.LastProcessedDataFileNumber);
	}
	fclose (pFile);
	ZeroProcessedDataArrays();
}


















