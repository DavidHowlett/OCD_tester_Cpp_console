//---------------------------------------------------------------------------
#include <stdio.h>
#include "ConsoleAlicatFlowmeter.h"
#include "Functions.h"
//---------------------------------------------------------------------------



#pragma package(smart_init)
//******************************************************************************
AlicatFlowmeter::AlicatFlowmeter(bool CreateSuspended):RS232Primitive(CreateSuspended)  //Constructor
//******************************************************************************
{
	LogCounter     = 0;                                         //Start putting the data into MeanSomething[0]
	FPressure      =-2;
	FTemperature   =-2;
	FVolumetricFlow=-2;
	FMassFlow      =-2;
	FGasType       ="None";
}
//******************************************************************************
void AlicatFlowmeter::ReadAndProcess()                         //Read the flowmeter data and update it
//******************************************************************************
{
	AnsiString s1;
	AnsiString s2;
	char sGasType[40];
	Read();                                                    //Read the flowmeter data
	s2=ReadUpTo(CRString);                                          //Get the string, if possible
	if (s2!="") {
		// Find last data line
		while (s2!="") {                                       //If this string was valid, try to get one more recent
			s1=s2;                                                 //Preserve the latest string we know to be goo
			s2=ReadUpTo(CRString);                                      //Try to get a new string from the port
		}
		//We can only get here when S2 is NULL, ie when we have flushed the buffer to the point of the last <CR>
		//We can only get here when S1 is not NULL.
		sscanf(s1.c_str(),"%f %f %f %f %s",&FPressure      ,
											&FTemperature   ,
											&FVolumetricFlow,
											&FMassFlow      ,
											&sGasType       );
		FGasType=sGasType;                                     //Wide to narrow string conversion
		if (FGasType!="Air") {
			FGasType="XXXXXX";
		}
		PressureLog      [LogCounter]=FPressure      ;         //Record of the pressures over time
		TemperatureLog   [LogCounter]=FTemperature   ;         //Record of the temperatures over time
		VolumetricFlowLog[LogCounter]=FVolumetricFlow;         //Record of the volumetric flows over time
		MassFlowLog      [LogCounter]=FMassFlow      ;         //Record of the mass flows over time
		if (++LogCounter>=MeanReadingsCount){                  //Update the LogCounter and if too high,
			LogCounter=0;                                        //reset to 0
		}
		TicksLastRead=GetTickCount(); //Capture the date/time of the data
	}
}

//******************************************************************************
Cardinal AlicatFlowmeter::DataAge()                              //Age of data in clock ticks
//******************************************************************************
{
	return (GetTickCount()-TicksLastRead);
}

//******************************************************************************
void AlicatFlowmeter::SetPressureMonitor      (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FPressureMonitor=Monitor;
}

//******************************************************************************
void AlicatFlowmeter::SetTemperatureMonitor   (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FTemperatureMonitor=Monitor;
}
//******************************************************************************
void AlicatFlowmeter::SetVolumetricFlowMonitor(TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FVolumetricFlowMonitor=Monitor;
}
//******************************************************************************
void AlicatFlowmeter::SetMassFlowMonitor      (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FMassFlowMonitor=Monitor;
}
//******************************************************************************
void AlicatFlowmeter::SetGasTypeMonitor       (TEdit* Monitor)   //Optionally use an Edit Box to monitor the output
//******************************************************************************
{
	FGasTypeMonitor=Monitor;
}
//******************************************************************************
float AlicatFlowmeter::Pressure      () //Return the pressure reading
//******************************************************************************
{
	return FPressure;
}

//******************************************************************************
float AlicatFlowmeter::Temperature   () //Return the temperature reading
//******************************************************************************
{
	return FTemperature;
}
//******************************************************************************
float AlicatFlowmeter::VolumetricFlow() //Return the volumetric flow reading
//******************************************************************************
{
	return FVolumetricFlow;
}
//******************************************************************************
float AlicatFlowmeter::MassFlow      () //Return the mass flow reading
//******************************************************************************
{
	return FMassFlow;
}
//******************************************************************************
String AlicatFlowmeter::GasType       () //Return the gas type
//******************************************************************************
{
	return FGasType;
}

//******************************************************************************
float AlicatFlowmeter::MeanPressure      () //Return the mean pressure reading
//******************************************************************************
{
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+PressureLog[i];
	}
	return Total/MeanReadingsCount;
}

//******************************************************************************
float AlicatFlowmeter::MeanTemperature   () //Return the mean temperature reading
//******************************************************************************
{
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+TemperatureLog[i];
	}
	return Total/MeanReadingsCount;
}
//******************************************************************************
float AlicatFlowmeter::MeanVolumetricFlow() //Return the mean volumetric flow reading
//******************************************************************************
{
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+VolumetricFlowLog[i];
	}
	return Total/MeanReadingsCount;
}
//******************************************************************************
float AlicatFlowmeter::MeanMassFlow      () //Return the mean mass flow reading
//******************************************************************************
{
	float Total=0;
	for (int i=0; i < MeanReadingsCount; i++) {
		Total=Total+MassFlowLog[i];
	}
	return Total/MeanReadingsCount;
}
//******************************************************************************
void AlicatFlowmeter::SetName()
//******************************************************************************
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = "FlowMeter";
	info.dwThreadID = -1;
	info.dwFlags = 0;

	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

//******************************************************************************
void __fastcall AlicatFlowmeter::UpdateMonitors()  //Update the monitor fields, if any
//Monitor fields, if set, are automatically updated 
//******************************************************************************
{
	char tmp[20];
	if (FPressureMonitor      !=NULL) {sprintf(tmp,"%f",Pressure()      );FPressureMonitor      ->Text=tmp      ;};
	if (FTemperatureMonitor   !=NULL) {sprintf(tmp,"%f",Temperature()   );FTemperatureMonitor   ->Text=tmp      ;};
	if (FVolumetricFlowMonitor!=NULL) {sprintf(tmp,"%f",VolumetricFlow());FVolumetricFlowMonitor->Text=tmp      ;};
	if (FMassFlowMonitor      !=NULL) {sprintf(tmp,"%f",MassFlow()      );FMassFlowMonitor      ->Text=tmp      ;};
	if (FGasTypeMonitor       !=NULL) {                                  ;FGasTypeMonitor       ->Text=GasType();};
}

//******************************************************************************
void __fastcall AlicatFlowmeter::Execute()
//******************************************************************************
{
	AnsiString s;
	SetName();
	//---- Place thread code here ----
	FreeOnTerminate = true;  //Free memory on termination
	while (!Terminated){
		Sleep(1);              //Don't cycle too fast- there's no point as there is not enough data.
		ReadAndProcess  (); //Obtain the data;
	}
}

