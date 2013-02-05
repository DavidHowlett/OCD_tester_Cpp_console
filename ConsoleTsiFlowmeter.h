#ifndef ConsoleTsiFlowmeterH
#define ConsoleTsiFlowmeterH

class TsiFlowmeter{
	public:
		TsiFlowmeter(int Port);
		bool CheckPresence();		  	// checks if the flowmeter answers to the string "?"
		void CallMeRegularly();     // this method should only exist for debugging
		float MassFlow();
		float Temperature();
		float Pressure();
		~TsiFlowmeter();
	private:
		int Write(char*);             	// sends a c string to the flowmeter
		void ClearBuffer();			// clears window's internal buffer
		int DataPointsFromTsi;
		float LastMassFlow;
};

#endif

