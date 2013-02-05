#ifndef ConsoleFlowmeterManagerH
#define ConsoleFlowmeterManagerH
#include "ConsoleSettingsFileManager.h"
class AlicatFlowmeter; // I chose to forward declare this rather then include the whole header file
class TsiFlowmeter;    // I chose to forward declare this rather then include the whole header file
//---------------------------------------------------------------------------
class FlowmeterManager{
	public:
		FlowmeterManager();
		int Setup(SettingsFileManager *); // this method should not finish until there is a valid flowmeter setup
		float MassFlow();
		~FlowmeterManager();

		bool FlowmeterReady; // this should be true if the flowmeter has been setup and the caller of this class can safely get data from it
		enum FlowmeterTypeEnum {None, Alicat, Tsi} FlowmeterType;
	private:
		bool TestPortExistence(int Port); // this checks if a port is a valid com port
		bool AttemptTsiSetup(int Port);
		bool AttemptAlicatSetup(int Port);

		#define MAX_PORT_NUMBER 200 // this is the largest port number the program can cope with
		bool PortExistence[MAX_PORT_NUMBER];
		AlicatFlowmeter* AlicatPointer;  // this creates a pointer of type: flowmeter class instance
		TsiFlowmeter* TsiPointer;
		SettingsFileManager* SettingsPointer;

};
#endif

