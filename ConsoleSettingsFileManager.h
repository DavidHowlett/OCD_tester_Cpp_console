#ifndef ConsoleSettingsFileManagerH
#define ConsoleSettingsFileManagerH
#include <stdio.h>
class SettingsFileManager
{
	// the purpose of this class is to store and manage all settings that must be stored in the settings file
	// I do this to aid encapsulation
	// rule for use of this settings file manager: whenever a setting is changed by the user of this class the write_file() method must be called as well.
	// I could have enforced this by forceing the modification of variables to happen through method calls but I decided against this due to the extra code required.
	private:
		FILE * FilePointer;
		static const int MaxSettings = 10;
		static const int MaxCharsInSettingDescriptor = 500;
		char 	SettingsDescriptors[MaxSettings][MaxCharsInSettingDescriptor];
	public:
		int		LatestGoodFlowmeterPort;
		int 	DataGatheringCycleTime; // this is in milliseconds
		int 	LastRawDataFileNumber;
		int 	LastProcessedDataFileNumber;
		float TriggerFlow; // this is the flow rate in standard litres per minute at which the software recognises that a pulse of air has started
		float AntibounceWaitTime;  // this is the length of time after the trigger flow has ben passed (upwards or downwards) after which the sysem is responsive to further changes
		float TimeToBeIncludedInPulseOnEitherSideOfTriggerTime;// The length of time on each side of the main body of the pulse that is included for the purposes of integrating to find the volume in the pulse
	public: // functions
		SettingsFileManager();
		~SettingsFileManager();
		int 	ReadFile();
		int 	WriteFile();
	private:
		int 	UnguardedReadFile();
		int 	UnguardedWriteFile();
};
#endif
