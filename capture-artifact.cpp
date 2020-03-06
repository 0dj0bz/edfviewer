#include <iostream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include "edftest.h"

struct EEGArtifactV1
{
	char version[20] = "EEGArtifactV1";
	int channel;
	int numsamples;
	char label[255];
} artheader;


// call with positional args:
// argv[1] : srcfile  - the EDF file containing the data
// argv[2] : dstfile  - the destination for the artifact data
// argv[2] : channel  - the signal channel to inspect
// argv[3] : startpos - the starting position (in seconds) of the artifact this notation represents
// argv[4] : endpos   - the ending position (in seconds) of the artifact this notation represents
// argv[5] : artlabel - the label for this artifact

int main(int argc, char **argv)
{

	if (argc < 8)
	{
		std::cout << "ERROR: not all required arguments provided." << std::endl;
		std::cout << "Usage:" << std::endl;
		std::cout << " edfsrcfile  - the FQN EDF file containing the data" << std::endl;
		std::cout << " tsesrcfile  - the FQN TSE file containing the labels" << std::endl;
		std::cout << " dstfile  - the FQN destination for the artifact data" << std::endl;
		std::cout << " channel  - the signal channel to inspect" << std::endl;
		std::cout << " startpos - the starting position (in seconds)" << std::endl;
		std::cout << "            of the artifact this notation represents" << std::endl;
		std::cout << " endpos   - the ending position (in seconds) of the " << std::endl;
		std::cout << "            artifact this notation represents" << std::endl;
		std::cout << " artlabel - the label for this artifact" << std::endl;
		exit(-1);
	}

	// save command line arguments to variables
	string edfsrcfile  = argv[1];
	string tsesrcfile  = argv[2];
	string dstfile     = argv[3];
	int channel        = stoi(argv[4]);
	float startpos     = stof(argv[5]);
	float endpos       = stof(argv[6]);
	string artlabel    = argv[7]; 

	EEGStudy *rstudy;

	std::cout << "Opening filename: " << edfsrcfile << std::endl;


	rstudy = loadEDFfile(edfsrcfile, false);

	if (rstudy == NULL)
	{
		std::cout << "Error opening file - ABORTING." << std::endl;
		exit(-2);
	}

	std::cout << "File duration (s) : " << stoi(rstudy->header->numDataRecs) * stoi(rstudy->header->recDuration) << std::endl;
	short * data; 

	if (startpos - 1.0f > 0.0f)
		startpos -= 1.0f;

	if (endpos +1.0f < stof(rstudy->header->recDuration)*stof(rstudy->header->numDataRecs))
		endpos += 1.0f;

	int numsamples = rstudy->getSegment(&data, channel, startpos, endpos);

	std::cout << "Num samples in segment: " << numsamples << std::endl;
	// std::cout << "Data: " << std::endl;

	// for (int i=0;i<numsamples;i++)
	// 	std::cout << "data[" << i << "]: " << data[i] << std::endl;

 	artheader.channel = channel;
	artheader.numsamples = numsamples;
	strcpy(artheader.label, artlabel.c_str());

	std::cout << "Writing file: " << dstfile << " channel: " << channel << " numsamples: " << numsamples <<
		" startpos: " << startpos << " endpos: " << endpos << std::endl;

	FILE *fp = fopen(dstfile.c_str(), "wb");
	// first, write the header
	fwrite(&artheader, sizeof(EEGArtifactV1), 1, fp );
	//now write the data, an array of shorts
	fwrite(data, sizeof(short), numsamples, fp);

	fflush(fp);
	fclose(fp);

	//now let's read it back in to see if it worked
	// EEGArtifactV1 test;

	// fp = fopen(dstfile.c_str(), "rb");
	// fread(&test, sizeof(EEGArtifactV1), 1, fp);
	// short indata[numsamples];
	// fread(indata, sizeof(short), numsamples, fp);
	// fclose(fp);

	// std::cout << "Header Data" << std::endl;
	// std::cout << "Version    : " << test.version << std::endl;
	// std::cout << "Channel    : " << test.channel << std::endl;
	// std::cout << "Label      : " << test.label << std::endl;
	// std::cout << "Numsamples : " << test.numsamples << std::endl << std::endl;

	// for (int i=0;i<numsamples;i++)
	// 	std::cout << "indata[" << i << "]: " << indata[i] << std::endl;

	// free the array of data
	free(data);

	std::cout << "Success! - exiting." << std::endl;
	return(0);
}