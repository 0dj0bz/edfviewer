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

struct EDF_SIGNAL
{
	char label[16+1];
	char transducerType[80+1];
	char physDimension[8+1];
	char physMinimum[8+1];
	char physMaximum[8+1];
	char digiMinimum[8+1];
	char digiMaximum[8+1];
	char prefilter[80+1];
	char numSamples[8+1];
	char reserved[32+1];
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


	EDFSignal operator=(EDFSignal rhs)
	{
		label = rhs.label.c_str();
		transducerType = rhs.transducerType.c_str();
		physDimension = rhs.physDimension.c_str();
		physMinimum = rhs.physMinimum.c_str();
		physMaximum = rhs.physMaximum.c_str();
		digiMinimum = rhs.digiMinimum.c_str();
		digiMaximum = rhs.digiMaximum.c_str();
		prefilter = rhs.prefilter.c_str();
		numSamples = rhs.numSamples.c_str();
		reserved = rhs.reserved.c_str();

	};

	~EDFSignal() 
	{
			
	};
		
};


void assign(EDF_SIGNAL *lhs, EDFSignal rhs)
{
	strcpy(&lhs->label[0], rhs.label.c_str());
	lhs->label[sizeof(lhs->label)-1] = '\0';

	strcpy(&lhs->transducerType[0], rhs.transducerType.c_str());
	lhs->transducerType[sizeof(lhs->transducerType)-1] = '\0';	

	strcpy(&lhs->physDimension[0], rhs.physDimension.c_str());
	lhs->physDimension[sizeof(lhs->physDimension)-1] = '\0';
	
	strcpy(&lhs->physMinimum[0], rhs.physMinimum.c_str());
	lhs->physMinimum[sizeof(lhs->physMinimum)-1] = '\0';

	strcpy(&lhs->physMaximum[0], rhs.physMaximum.c_str());
	lhs->physMaximum[sizeof(lhs->physMaximum)-1] = '\0';

	strcpy(&lhs->digiMinimum[0], rhs.digiMinimum.c_str());
	lhs->digiMinimum[sizeof(lhs->digiMinimum)-1] = '\0';

	strcpy(&lhs->digiMaximum[0], rhs.digiMaximum.c_str());
	lhs->digiMaximum[sizeof(lhs->digiMaximum)-1] = '\0';

	strcpy(&lhs->prefilter[0], rhs.prefilter.c_str());
	lhs->prefilter[sizeof(lhs->prefilter)-1] = '\0';

	strcpy(&lhs->numSamples[0], rhs.numSamples.c_str());
	lhs->numSamples[sizeof(lhs->numSamples)-1] = '\0';

	strcpy(&lhs->reserved[0], rhs.reserved.c_str());
	lhs->reserved[sizeof(lhs->reserved)-1] = '\0';

};



class EDFData
{
public:
	short ***data;
	long maxSamples;

	EDFData(int signals, int numRecs, int sampleFreq)
	{
		maxSamples = numRecs;

		data = (short ***) malloc((numRecs+1)*sizeof(short *));
		if (data)
		{
			for (int i=0;i<=signals;i++)
			{
				data[i] = (short **) malloc((numRecs+1)*sizeof(short *));
				if (data[i])
				{
					for (int j=0;j<=sampleFreq;j++)
					{
						data[i][j] = (short *) malloc((sampleFreq+1)*sizeof(short *));

						if (!data[i][j])
						{
							std::cout << "EDFData(int, int, int): error alocating space. (3)" << std::endl;
						}
					}
				}
				else
				{
					std::cout << "EDFData(int,int,int): error allocating space. (2)" << std::endl;

				}
				
			}
		}
		else
			std::cout << "EDFData(int,int,int): error allocating space (1)" << std::endl;

	};

};

class EEGStudy
{
public:
	EDFHeader *header;
	std::map<int, EDFSignal> signals;
	short ***signalData;


	EEGStudy()
	{
		this->header = NULL;
		this->signalData = NULL;
	};

	EEGStudy(EDFHeader *hdr, std::map<int, EDFSignal> sigList, short ***sigData)
	{
		this->header = hdr;
		this->signals = sigList;
		this->signalData = sigData;
	};


	~EEGStudy()
	{

	
		for (int i=0;i<stoi(this->header->numDataRecs);i++)
		{

			if (this->signalData[i]) // malloc successful
			{	

				for (int j=0;j<stoi(this->header->numSignals);j++)
				{
					free(this->signalData[i][j]);
				}

			}

			free(this->signalData[i]);
		}
		
		free(this->signalData);

		delete this->header;

	};

	// EEGStudy::getSegment(short *dst, int sigNum, float startTime, float endTime)
	// Assumptions:
	// short *dst has been malloc'd large enough to hold 1+(endTime-startTime)*freq short ints
	// endTime >= startTime
	// sigNum < this->header->numSignals

	int getSegment(short * &dst, int sigNum, float startTime, float endTime)
	{

		int freq = stoi(this->signals[sigNum].numSamples) / stoi(this->header->recDuration);

		startTime = floor(startTime);
		endTime = ceil(endTime);

		int startPos = freq * startTime;
		int endPos = freq * endTime;

		std::cout << "EEGStudy::getSegment - startTime: " << startTime << std::endl;
		std::cout << "EEGStudy::getSegment - endTime  : " << endTime << std::endl;
				
		std::cout << "EEGStudy::getSegment - startPos : " << startPos  << std::endl;
		std::cout << "EEGStudy::getSegment - endPos   : " << endPos  << std::endl;

		// std::cout << "About to create dst array...";

				
		long numSamples = endPos-startPos;
		dst = (short *) malloc(numSamples*sizeof(short));

		long samplesToCopy = numSamples;
		
		if (dst)
		{
			long curRec = 0;
			
			int startRecord = floor(startTime)/stol(this->header->recDuration);

			while (samplesToCopy>0)
			{
				// to do this correctly, we MAY need to execute multiple memcpy statements:
				//
				// there may be one or more records in this->signalData->data that contain this data
				//
				// each record may be indexed by:
				//	floor(startTime/this->header->recDuration)
				//
				// since we may start part-way into a record, the address of the loction is found
				// by 
				//	mod(startTime/this->header->recDuration)
				//
				// and the number of short ints to copy is found by
				//	this->signals[sigNum].numSamples - mod(startTime/this->header->recDuration) + 1
				//
				// this process continues until samplesRemaining == 0.

				// for this time, we will start the range with the floor() of the startTime and
				// the ceil() of the endTime to make the processing easier. we may come back later and
				// implement partial record ranges. - rla 2020/08/18

				//std::cout << "EEGStudy::getSegment - about to copy data to dst array..." << std::endl;

				int startRecord = floor(startTime)/stol(this->header->recDuration);

				memcpy(dst+(curRec*freq), this->signalData[startRecord+curRec][sigNum],
					stol(this->signals[sigNum].numSamples)*sizeof(short));
					//(endPos-startPos+1)*sizeof(short));

				samplesToCopy -= stol(this->signals[sigNum].numSamples);

				curRec++;

				//std::cout << "EEGStudy::getSegment - SUCCESS! Copied " << (endPos-startPos) << " values." 
					//<< std::endl;

			 	//std::cout << "EEGStudy::getSegment - samples remaining: " << samplesToCopy << std::endl; 
			}
		}	
		else
		{
		 	std::cout << "EEGStudy::getSegment - malloc failed." << std::endl;
		 	return(-1);
		}

		return((endPos-startPos));

	}

	void loadEDFfile(string fn, bool verbose=false)
	{
	
		enum FileType {EDF, EDFPLUS};
//		EDFHeader * header;
//		std::map<int, EDFSignal> signalList;
//		EDFData * edfData;


		FILE* f = fopen(fn.c_str(), "r");

		if (f == NULL)
		{

			std::cout << "Error opening file - ABORTING." << std::endl;
			return;
		}
		else
			std::cout << "File opened successfully." << std::endl;

		// Process the fixed part of the header (first 256 bytes)
 
		fread(&hdr, sizeof(hdr), 1, f);

		this->header = new EDFHeader();

		this->header->patientId   = string(hdr.patientId,   sizeof(hdr.patientId));
		this->header->startDate   = string(hdr.startDate,   sizeof(hdr.startDate));
		this->header->startTime   = string(hdr.startTime,   sizeof(hdr.startTime));
		this->header->reserved    = string(hdr.reserved,    sizeof(hdr.reserved));
		this->header->numDataRecs = string(hdr.numDataRecs, sizeof(hdr.numDataRecs));
		this->header->recDuration = string(hdr.recDuration, sizeof(hdr.recDuration));
		this->header->numSignals  = string(hdr.numSignals,  sizeof(hdr.numSignals));

		if (verbose)
		{
			cout << "FileType : " << ((this->header->reserved.substr(0, 4)=="EDF+")?"EDF+":"EDF") << endl;

			cout << "EDF File : " << fn << endl;
			cout << "-----------------------------------------------------------------------------------------------------" << endl;
			cout << "PatientId           : " << this->header->patientId   << endl;
			cout << "Start Date          : " << this->header->startDate   << endl;
			cout << "Start Time          : " << this->header->startTime   << endl;
			cout << "Reserved            : " << this->header->reserved    << endl;
			cout << "# Data Recs         : " << this->header->numDataRecs << endl;
			cout << "Record Duration (s) : " << this->header->recDuration << endl;
			cout << "Number of Signals   : " << this->header->numSignals  << endl;
		}

		tm tm_sessionTime;

		tm_sessionTime.tm_mday = stoi(this->header->startDate.substr(0, 2));
		tm_sessionTime.tm_mon  = stoi(this->header->startDate.substr(3, 2))-1;
		tm_sessionTime.tm_year = stoi(this->header->startDate.substr(6, 2))+100;

		tm_sessionTime.tm_hour = stoi(this->header->startTime.substr(0, 2));
		tm_sessionTime.tm_min  = stoi(this->header->startTime.substr(3, 2));
		tm_sessionTime.tm_sec  = stoi(this->header->startTime.substr(6, 2));
		tm_sessionTime.tm_isdst= -1; // this is -1 since TZ & DST not available in EDF files

		time_t t_sessionTime = mktime(&tm_sessionTime); 

		double recordDuration = stod(this->header->recDuration);

		// now we have enough information to process the variable part of the header

		long numSignals = stol(this->header->numSignals);


		// first, the labels

		LABEL labels[numSignals];
		fread(&labels, sizeof(LABEL), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].label = string(labels[i].value, sizeof(LABEL));
		}

		// next, transducer types

		TRANSDUCER_TYPE ttypes[numSignals];
		fread(&ttypes, sizeof(TRANSDUCER_TYPE), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].transducerType = string(ttypes[i].value, sizeof(TRANSDUCER_TYPE));
		}

		// next, physical dimension of each signal

		PHYS_DIMENSION physDimension[numSignals];
		fread(&physDimension, sizeof(PHYS_DIMENSION), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].physDimension = string(physDimension[i].value, sizeof(PHYS_DIMENSION));
		}

		// next, physical minimum

		VALUE physMinimum[numSignals];
		fread(&physMinimum, sizeof(VALUE), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].physMinimum = string(physMinimum[i].value, sizeof(VALUE));
		}

		// next, physical maximum

		VALUE physMaximum[numSignals];
		fread(&physMaximum, sizeof(VALUE), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].physMaximum = string(physMaximum[i].value, sizeof(VALUE));
		}

		// next, digital minimum

		VALUE digiMinimum[numSignals];
		fread(&digiMinimum, sizeof(VALUE), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].digiMinimum = string(digiMinimum[i].value, sizeof(VALUE));
		}

		// next, digital maximum

		VALUE digiMaximum[numSignals];
		fread(&digiMaximum, sizeof(VALUE), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].digiMaximum = string(digiMaximum[i].value, sizeof(VALUE));
		}

		// next, prefiltering

		PREFILTER prefilter[numSignals];
		fread(&prefilter, sizeof(PREFILTER), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].prefilter = string(prefilter[i].value, sizeof(VALUE));
		}

		// next, samples / record

		SAMPLE_COUNT numSamples[numSignals];
		fread(&numSamples, sizeof(SAMPLE_COUNT), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].numSamples = string(numSamples[i].value, sizeof(SAMPLE_COUNT));
		}

		// next, reserved

		RESERVED reserved[numSignals];
		fread(&reserved, sizeof(RESERVED), numSignals, f);

		for (int i=0; i<numSignals; i++)
		{
			signals[i].reserved = string(reserved[i].value, sizeof(RESERVED));
		}

		if (verbose)
		{
			cout << "-------------------------------------------------------------------------------------------------" << endl;

			for (int i=0; i<numSignals; i++)
			{

				cout << "Sample        : " << i << endl;
				cout << "Label         : " << signals[i].label << endl;
				cout << "Trans. Type   :" << signals[i].transducerType << endl;
				cout << "Phys. Dim.    : " << signals[i].physDimension << " Min : " << signals[i].physMinimum << " Max : " << 
				signals[i].physMaximum << endl;
				cout << "Digital Min   : " << signals[i].digiMinimum << " Max : " << signals[i].digiMaximum << endl;
				cout << "Prefilter     : " << signals[i].prefilter << endl;
				cout << "Samples / rec : " << signals[i].numSamples << endl;
				cout << "Reserved      : " << signals[i].reserved << endl;

				cout << "-------------------------------------------------------------------------------------------------" << endl;

			}
		}

		// now, read the data
		// the file will have header.numDataRecs "rows" of data, each with header.numSignals sets of data elements
		// for each signal s in the data rec, there will be signalList[s].numSamples of 2-byte (short int) values
		// 
		// the timestamp for each data rec will be header.startTime [hh:mm:ss] + (recNum*header.recDuration) seconds
		// so, data should be allocated using numDataRecs * max(numSamples)
	
		t_sessionTime = mktime(&tm_sessionTime); 

		//this->signalData = new EDFData();

		short *** data;

		this->signalData = (short ***) malloc(stol(this->header->numDataRecs)*sizeof(short **));

		for (int i=0;i<stoi(this->header->numDataRecs);i++)
		{
			// for each rec, allocate an array of pointers to store each block of signal data

    			this->signalData[i] = (short **) malloc(stol(this->header->numSignals)*sizeof(short *));

			if (this->signalData[i]) // malloc successful
			{	
				if (verbose)
    				{
			    		cout << "-------------------------------------------------------------------------------------------------" << endl;

			    		// compute the timestamp for this data record
					cout << asctime(localtime(&t_sessionTime)) << endl;
				}


				struct tm *ptm_sessionTime = localtime(&t_sessionTime);
				ptm_sessionTime->tm_sec += stod(this->header->recDuration);
		    		t_sessionTime = mktime(ptm_sessionTime);

				// for each signal in the record, read the number of samples

				for (int j=0;j<stoi(this->header->numSignals);j++)
				{
					// now for each channel, we need to allocate a block of data to read the data into (signalList[j].numSamples) 
			
					this->signalData[i][j] = (short *) malloc((stol(this->signals[j].numSamples)+1)*sizeof(short));


	    				fread(this->signalData[i][j], sizeof(short), stoi(this->signals[j].numSamples), f);

				    	if (verbose)
				    	{
				    		if (j==0)
						{
						    	cout << "Signal[" << j << "] Data Record[" << i << "]\t - ";
							for (int k=0;k<stoi(this->signals[j].numSamples);k++)
						    	cout << this->signalData[i][j][k] << ", " << endl;
						}

				    	}

				}

				if (verbose)
					cout << endl;
			}
		}


//		study = new EEGStudy(header, signalList, edfData);

		if (verbose)
		{
			for (int j=0;j<stoi(this->header->numSignals);j++)
			    for (int i=0;i<stoi(this->header->numDataRecs);i++)
				for (int k=0;k<stoi(this->signals[j].numSamples);k++)
				    	cout << "Signal[" << j << "] Data Record[" << i << "]elem[" << k << "]\t - " << this->signalData[j][i][k] << endl;
		}

//		return study;
	}
};
