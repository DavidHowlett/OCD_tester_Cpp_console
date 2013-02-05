#include <iostream.h>
#include "ConsoleTsiFlowmeter.h"
#include <ExtCtrls.hpp>

HANDLE TsiPortHandle;

TsiFlowmeter::TsiFlowmeter(int Port){

	const size_t Newsize = 100;
	wchar_t MyWideString[Newsize];
	swprintf(MyWideString,Newsize, L"\\\\.\\COM%d",Port); // note that this is the wide char equivelent of sprintf, the capital L tells the program that input string should be wide char
	TsiPortHandle = CreateFile(				MyWideString,  	            //LPCTSTR lpFileName,
												GENERIC_READ | GENERIC_WRITE, //DWORD dwDesiredAccess ,
												0,	                  //DWORD dwShareMode (0 for COM port access),
												SECURITY_ANONYMOUS,	  //LPSECURITY_ATTRIBUTES lpSecurityAttributes,
												OPEN_EXISTING,	      //DWORD dwCreationDisposition (necessary to be OPEN_EXISTING for COM ports),
//												FILE_FLAG_OVERLAPPED,  //DWORD dwFlagsAndAttributes,
												FILE_ATTRIBUTE_NORMAL,//DWORD dwFlagsAndAttributes,
												0);	                  //HANDLE hTemplateFile
	if ((TsiPortHandle==INVALID_HANDLE_VALUE)||(TsiPortHandle==NULL)){
		CloseHandle(TsiPortHandle);
		cout << "The tsi flowmeter class was passed the number of a port that it could not open." << endl;
	}
	DCB dcbCommPort;   //This structure is used to hold the COM port parameters
	dcbCommPort.DCBlength = sizeof(DCB); //This never needs to be changed, so set it on startup
	GetCommState(TsiPortHandle, &dcbCommPort);  //Read whatever state is to avoid hComPort's random contents corrupting defaults

	dcbCommPort.BaudRate=38400;
	dcbCommPort.fParity =1;
	dcbCommPort.Parity  =0;
	dcbCommPort.ByteSize=8;
	dcbCommPort.StopBits=0; // this means 1 stop bit

	SetCommState(TsiPortHandle, &dcbCommPort); //Write the modified result back.

	COMMTIMEOUTS CommTimeouts;
	CommTimeouts.ReadIntervalTimeout          =1;
	CommTimeouts.ReadTotalTimeoutConstant     =1;
	CommTimeouts.ReadTotalTimeoutMultiplier   =1;
	CommTimeouts.WriteTotalTimeoutConstant    =1;
	CommTimeouts.WriteTotalTimeoutMultiplier  =1;

	SetCommTimeouts(TsiPortHandle,&CommTimeouts);

}
void TsiFlowmeter::CallMeRegularly(){
	Write("DAFxx0001");
	Sleep(12); // by experiment this was found to be sufficent time to get data
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	ReadFile(		TsiPortHandle,                        //HANDLE        hFile,
					Buffer,                          //LPVOID        lpBuffer,
					BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					FALSE);
	sscanf(Buffer,"OK%f",&LastMassFlow);
}
float TsiFlowmeter::MassFlow(){
	CallMeRegularly();
	return LastMassFlow;
}

float TsiFlowmeter::Temperature(){
	return 0;
}
float TsiFlowmeter::Pressure(){
	return 0;
}
int TsiFlowmeter::Write(char* ToSend){             	// sends a c string to the flowmeter
	DWORD BytesWritten;					   //Number of bytes written
	AnsiString AStringSendMe(ToSend), CarrageReturn('\r'); // a carrage return is needed for the tsi flowmeter to recognise a command.
	AStringSendMe += CarrageReturn;
	//printf(AStringSendMe.c_str()); // remove me
	ClearBuffer();// this clears window's internal buffer
	WriteFile(					TsiPortHandle,                  //HANDLE hComFile
								AStringSendMe.c_str(),                  //LPCVOID lpBuffer,
								AStringSendMe.Length(),                  //DWORD nNumberOfBytesToWrite,
								&BytesWritten ,                  //LPDWORD lpNumberOfBytesWritten,
								FALSE);                  //LPOVERLAPPED lpOverlapped
	return 0;
}
bool TsiFlowmeter::CheckPresence(){		  	// checks if the flowmeter answers to the string "?"
	Write("?");
	Sleep(30);
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	ReadFile(		TsiPortHandle,                        //HANDLE        hFile,
					Buffer,                          //LPVOID        lpBuffer,
					BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					FALSE);
	if('O'==Buffer[0] && 'K'==Buffer[1])
		return true;
	else
		return false;
}
void TsiFlowmeter::ClearBuffer(){			// clears window's internal buffer of data sent from the board
	unsigned long BytesRead;
	const int BufferSize=200;
	char Buffer[BufferSize];
	ReadFile(		TsiPortHandle,                        //HANDLE        hFile,
					Buffer,                          //LPVOID        lpBuffer,
					BufferSize,                    //DWORD         nNumberOfBytesToRead,
					&BytesRead,                       //LPDWORD       lpNumberOfBytesRead,
					FALSE);
}
TsiFlowmeter::~TsiFlowmeter(){
	CloseHandle(TsiPortHandle);
}
