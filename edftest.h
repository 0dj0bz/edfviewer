#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <cstdio>
#include <vector>
#include <map>
#include <chrono>
#include <ctime>


using namespace std;

struct EDF_HEADER
{
	char version[8];
	char patientId[80];
	char localId[80];
	char startDate[8];
	char startTime[8];
	char headerBytes[8];
	char reserved[44];
	char numDataRecs[8];
	char recDuration[8];
	char numSignals[4];
} hdr;



struct LABEL
{
	char value[16];
};

struct TRANSDUCER_TYPE
{
	char value[80];
};

struct PHYS_DIMENSION
{
	char value[8];
};

struct VALUE
{
	char value[8];
};

struct PREFILTER
{
	char value[80];
};

struct SAMPLE_COUNT
{
	char value[8];
};

struct RESERVED
{
	char value[32];
};

class EDFHeader
{
public:
	string version;
	string patientId;
	string localId;
	string startDate;
	string startTime;
	string headerBytes;
	string reserved;
	string numDataRecs;
	string recDuration;
	string numSignals;
};

class EDFSignal
{
public:
	string label;
	string transducerType;
	string physDimension;
	string physMinimum;
	string physMaximum;
	string digiMinimum;
	string digiMaximum;
	string prefilter;
	string numSamples;
	string reserved;
};

class EDFData
{
public:
	short **data;

	EDFData(int signals, int numRecs)
	{
		data = (short **) malloc(signals*sizeof(short *));
		for (int i=0;i<numRecs;i++)
			data[i] = (short *) malloc(numRecs*sizeof(short));
	};

};

class EEGStudy
{
public:
	EDFHeader *header;
	std::map<int, EDFSignal> *signals;
	EDFData *signalData;

	EEGStudy(EDFHeader *hdr, std::map<int, EDFSignal> *sigList, EDFData *sigData)
	{
		this->header = hdr;
		this->signals = sigList;
		this->signalData = sigData;
	}

	short * getSegment(int sigNum, float startTime, float endTime)
	{

		int freq = stoi(this->signals[0][sigNum].numSamples) / stoi(this->header->recDuration);

		int startPos = freq * startTime;
		int endPos = freq * endTime;

		short * sel = new short[(endPos-startPos)+1];

		memcpy(sel, &this->signalData->data[sigNum][startPos], (endPos-startPos)+1);

		return(sel);

	}
};

enum FileType {EDF, EDFPLUS};

EEGStudy * study;
EDFHeader * header;
std::map<int, EDFSignal> signalList;
EDFData * data;


EEGStudy * loadEDFfile(string fn, bool verbose=false)
{
	
	// if (fn == NULL)
	// {
	// 	cout << "Usage: edftest <filename>" << endl;
	// 	exit(-1);
	// }

	FILE* f = fopen(fn.c_str(), "r");

	if (f == NULL)
	{

		std::cout << "Error opening file - ABORTING." << std::endl;
		return(NULL);
	}
	else
		std::cout << "File opened successfully." << std::endl;

	// Process the fixed part of the header (first 256 bytes)
 
    fread(&hdr, sizeof(hdr), 1, f);

    header = new EDFHeader();

    header->patientId   = string(hdr.patientId,   sizeof(hdr.patientId));
    header->startDate   = string(hdr.startDate,   sizeof(hdr.startDate));
    header->startTime   = string(hdr.startTime,   sizeof(hdr.startTime));
    header->reserved    = string(hdr.reserved,    sizeof(hdr.reserved));
    header->numDataRecs = string(hdr.numDataRecs, sizeof(hdr.numDataRecs));
    header->recDuration = string(hdr.recDuration, sizeof(hdr.recDuration));
    header->numSignals  = string(hdr.numSignals,  sizeof(hdr.numSignals));

    if (verbose)
    {
	 	cout << "FileType : " << ((header->reserved.substr(0, 4)=="EDF+")?"EDF+":"EDF") << endl;

	 	cout << "EDF File : " << fn << endl;
	 	cout << "-----------------------------------------------------------------------------------------------------" << endl;
	    cout << "PatientId           : " << header->patientId   << endl;
	    cout << "Start Date          : " << header->startDate   << endl;
	    cout << "Start Time          : " << header->startTime   << endl;
	    cout << "Reserved            : " << header->reserved    << endl;
	    cout << "# Data Recs         : " << header->numDataRecs << endl;
	    cout << "Record Duration (s) : " << header->recDuration << endl;
	    cout << "Number of Signals   : " << header->numSignals  << endl;
	}

	tm tm_sessionTime;

	tm_sessionTime.tm_mday = stoi(header->startDate.substr(0, 2));
	tm_sessionTime.tm_mon  = stoi(header->startDate.substr(3, 2))-1;
	tm_sessionTime.tm_year = stoi(header->startDate.substr(6, 2))+100;

	tm_sessionTime.tm_hour = stoi(header->startTime.substr(0, 2));
	tm_sessionTime.tm_min  = stoi(header->startTime.substr(3, 2));
	tm_sessionTime.tm_sec  = stoi(header->startTime.substr(6, 2));
	tm_sessionTime.tm_isdst= -1; // this is -1 since TZ & DST not available in EDF files

	time_t t_sessionTime = mktime(&tm_sessionTime); 

	double recordDuration = stod(header->recDuration);

    // now we have enough information to process the variable part of the header

	//signalList = new std::map<int, EDFSignal>;
    long numSignals = stol(string(hdr.numSignals, sizeof(hdr.numSignals)));


    // first, the labels

    LABEL labels[numSignals];
    fread(&labels, sizeof(LABEL), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].label = string(labels[i].value, sizeof(LABEL));
    }

    // next, transducer types

    TRANSDUCER_TYPE ttypes[numSignals];
    fread(&ttypes, sizeof(TRANSDUCER_TYPE), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].transducerType = string(ttypes[i].value, sizeof(TRANSDUCER_TYPE));
    }

    // next, physical dimension of each signal

    PHYS_DIMENSION physDimension[numSignals];
    fread(&physDimension, sizeof(PHYS_DIMENSION), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].physDimension = string(physDimension[i].value, sizeof(PHYS_DIMENSION));
    }

    // next, physical minimum

    VALUE physMinimum[numSignals];
    fread(&physMinimum, sizeof(VALUE), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].physMinimum = string(physMinimum[i].value, sizeof(VALUE));
    }

    // next, physical maximum

    VALUE physMaximum[numSignals];
    fread(&physMaximum, sizeof(VALUE), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].physMaximum = string(physMaximum[i].value, sizeof(VALUE));
    }

    // next, digital minimum

    VALUE digiMinimum[numSignals];
    fread(&digiMinimum, sizeof(VALUE), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].digiMinimum = string(digiMinimum[i].value, sizeof(VALUE));
    }

    // next, digital maximum

    VALUE digiMaximum[numSignals];
    fread(&digiMaximum, sizeof(VALUE), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].digiMaximum = string(digiMaximum[i].value, sizeof(VALUE));
    }

    // next, prefiltering

    PREFILTER prefilter[numSignals];
    fread(&prefilter, sizeof(PREFILTER), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].prefilter = string(prefilter[i].value, sizeof(VALUE));
    }

    // next, samples / record

    SAMPLE_COUNT numSamples[numSignals];
    fread(&numSamples, sizeof(SAMPLE_COUNT), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].numSamples = string(numSamples[i].value, sizeof(SAMPLE_COUNT));
    }

    // next, reserved

    RESERVED reserved[numSignals];
    fread(&reserved, sizeof(RESERVED), numSignals, f);

    for (int i=0; i<numSignals; i++)
    {
    	signalList[i].reserved = string(reserved[i].value, sizeof(RESERVED));
    }

    if (verbose)
    {
	 	cout << "-------------------------------------------------------------------------------------------------" << endl;

	    for (int i=0; i<numSignals; i++)
	    {

	    	cout << "Sample        : " << i << endl;
	    	cout << "Label         : " << signalList[i].label << endl;
	    	cout << "Trans. Type   :" << signalList[i].transducerType << endl;
	    	cout << "Phys. Dim.    : " << signalList[i].physDimension << " Min : " << signalList[i].physMinimum << " Max : " << 
	    		signalList[i].physMaximum << endl;
	    	cout << "Digital Min   : " << signalList[i].digiMinimum << " Max : " << signalList[i].digiMaximum << endl;
	    	cout << "Prefilter     : " << signalList[i].prefilter << endl;
	    	cout << "Samples / rec : " << signalList[i].numSamples << endl;
	    	cout << "Reserved      : " << signalList[i].reserved << endl;

	    	cout << "-------------------------------------------------------------------------------------------------" << endl;

	    }
	}

    // now, read the data
    // the file will have header.numDataRecs "rows" of data, each with header.numSignals sets of data elements
    // for each signal s in the data rec, there will be signalList[s].numSamples of 2-byte (short int) values
    // 
    // the timestamp for each data rec will be header.startTime [hh:mm:ss] + (recNum*header.recDuration) seconds

	t_sessionTime = mktime(&tm_sessionTime); 

    data = new EDFData(stoi(header->numSignals), stoi(header->numDataRecs));

    for (int i=0;i<stoi(header->numDataRecs);i++)
    {

    	if (verbose)
    	{
	    	cout << "-------------------------------------------------------------------------------------------------" << endl;

	    	// compute the timestamp for this data record
			cout << asctime(localtime(&t_sessionTime)) << endl;
		}

		struct tm *ptm_sessionTime = localtime(&t_sessionTime);
		ptm_sessionTime->tm_sec += stod(header->recDuration);
    	t_sessionTime = mktime(ptm_sessionTime);

		// for each signal in the record, read the number of samples



		for (int j=0;j<stoi(header->numSignals);j++)
		{
			short samples[stoi(signalList[j].numSamples)];
			//cout << "Signal : " << j << " numSamples: " << stoi(signalList[j].numSamples) << endl;

    		fread(&samples, sizeof(short), stoi(signalList[j].numSamples), f);

	    	if (verbose)
	    	{
	    		if (j==0)
	    			//cout << "Data Record[" << i << "] Signal[" << j << "] \t - " << stoi(signalList[j].numSamples) << " samples read." << endl;
			    	cout << "Signal[" << j << "] Data Record[" << i << "]\t - " << samples[j] << endl;

	    	}

	    	data->data[j][i] = samples[j];
		}

		if (verbose)
			cout << endl;
    }

    study = new EEGStudy(header, &signalList, data);

    if (verbose)
    {
		for (int j=0;j<stoi(study->header->numSignals);j++)
		    for (int i=0;i<stoi(study->header->numDataRecs);i++)
		    	cout << "Signal[" << j << "] Data Record[" << i << "]\t - " << study->signalData->data[j][i] << endl;
	}

	return study;
}