#include "ConsoleTsiFlowmeter.h"
#include "ConsoleAlicatFlowmeter.h"
#include "ConsoleFlowmeterManager.h"
#include <iostream.h>
HANDLE aComFile;
//---------------------------------------------------------------------------
FlowmeterManager::FlowmeterManager(){
	FlowmeterReady = false;
	FlowmeterType = None;
}
int FlowmeterManager::Setup(SettingsFileManager * GivenSettingsPointer){ // this function returns 0 when it sucsessfuly sets a flowmeter up
	SettingsPointer=GivenSettingsPointer;
	int PortNum;
// the flowmeter port does not change often so first try the one that worked last time
	PortNum = SettingsPointer->LatestGoodFlowmeterPort;
	if (TestPortExistence(PortNum)){
		if(AttemptTsiSetup(PortNum))
			return 0;
		if(AttemptAlicatSetup(PortNum))
			return 0;

	}
// now systematically try all the ports
	cout << "Searching ports for TSI flowmeter" << endl;
	for (PortNum = 0;(PortNum < MAX_PORT_NUMBER) && (!FlowmeterReady); PortNum++) {
		if(TestPortExistence(PortNum)){
			PortExistence[PortNum] =true;
			cout << PortNum << endl;
			if(AttemptTsiSetup(PortNum))
				return 0;
		}else{
			PortExistence[PortNum] =false;
		}
	}
	cout << "Searching ports for Alicat flowmeter" << endl;
	for (PortNum = 0;(PortNum < MAX_PORT_NUMBER) && (!FlowmeterReady); PortNum++) { // I seperate the alicat setup attempts from the tsi setup attempts because windows takes a little bit ro relace a COM port
		if(PortExistence[PortNum]){
			cout << PortNum << endl;
			if(AttemptAlicatSetup(PortNum))
				return 0;
		}
	}
	if(!FlowmeterReady)
		cout << "Warning: no flowmeter found" << endl;
	return 1;
}
bool FlowmeterManager::TestPortExistence(int Port){// it would save the time of the user if the most recent sucsessfull flowmeter port detection was recorded in the settings file and then checked first
	const size_t NewSize = 100;
	wchar_t MyWideString[NewSize];
	swprintf(MyWideString,NewSize, L"\\\\.\\COM%d",Port); // note that this is the wide char equivelent of sprintf, the capital L tells the program that input string should be wide char
	aComFile = CreateFile(					   	MyWideString,  	            //LPCTSTR lpFileName,
												GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
												0,	                  //DWORD dwShareMode (0 for COM port access),
												SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
												OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
//												FILE_FLAG_OVERLAPPED,  //DWORD dwFlagsAndAttributes,
												FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
												0);	                  //HANDLE hTemplateFile
	if ((aComFile==INVALID_HANDLE_VALUE)||(aComFile==NULL)){
		CloseHandle(aComFile);
		return false;
	}else{
		CloseHandle(aComFile);
		return true;
	}
}
bool FlowmeterManager::AttemptTsiSetup(int Port){
	TsiPointer = new TsiFlowmeter(Port);
	if (TsiPointer->CheckPresence()) {
		FlowmeterReady = true;
		FlowmeterType = Tsi;
		SettingsPointer->LatestGoodFlowmeterPort = Port;
		SettingsPointer->WriteFile();
		cout << "TSI flowmeter found on port " << Port << endl;
		return true;
	}else{
		//cout << "TSI flowmeter not found on port " << port << endl; // remove me later
		delete TsiPointer;
		return false;
	}
}
bool FlowmeterManager::AttemptAlicatSetup(int Port){// this function is evil because it starts a class and does not close it
	char Name[10];
	sprintf(Name,"COM%d",Port);
	AlicatPointer = new AlicatFlowmeter(True);
	AlicatPointer->Setup(Name,38400,'N',8,1);             //COM port, baud rate, parity, bytes per bit, stop bits
	AlicatPointer->Open();                                   //Enable port access
	AlicatPointer->Resume();
	Sleep(100); // this alows the flowmeter class to get some data so it can be used safely
	if (AlicatPointer->MassFlow()!= -2){                                // this never seems to be null
		FlowmeterReady = true;
		FlowmeterType = Alicat;
		SettingsPointer->LatestGoodFlowmeterPort = Port;
		SettingsPointer->WriteFile();
		cout << "Alicat flowmeter found on port " << Port << endl;
		return true;
	}else{
		AlicatPointer->Close();
		return false;
	}
}
float FlowmeterManager::MassFlow(){
	if (Alicat == FlowmeterType) {
		return(AlicatPointer->MassFlow());
	}
	if (Tsi == FlowmeterType) {
		return(TsiPointer->MassFlow());
	}
	cout << "Flowmeter should be setup before readings are taken" << endl;
	return 0;
}
FlowmeterManager::~FlowmeterManager(){
	delete AlicatPointer;
	delete TsiPointer;
}

