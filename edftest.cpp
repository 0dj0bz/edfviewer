#include "edftest.h"

int main(int argc, char const *argv[])
{
	EEGStudy *rstudy = loadEDFfile("00000000_s001_t000.edf", false);

//	std::cout << "numSignals: " << rstudy->header->numSignals << std::endl;

	std::cout << "Inside main..." << std::endl;
	
	for (int j=0;j<stoi(rstudy->header->numSignals);j++)
	    for (int i=0;i<stoi(rstudy->header->numDataRecs);i++)
	    	cout << "Signal[" << j << "] Data Record[" << i << "]\t - " << rstudy->signalData->data[j][i] << endl;

	return 0;
}