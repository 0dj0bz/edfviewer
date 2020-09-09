#include <iostream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include "edftest.h"



// call with positional args:
// argv[1] : srcfile  - the EDF file containing the data
// argv[2] : dstfile  - the destination for the artifact data
// argv[2] : channel  - the signal channel to inspect
// argv[3] : startpos - the starting position (in seconds) of the artifact this notation represents
// argv[4] : endpos   - the ending position (in seconds) of the artifact this notation represents
// argv[5] : artlabel - the label for this artifact

int main(int argc, char **argv) {

    if (argc < 8) {
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
    string edfsrcfile = argv[1];
    string tsesrcfile = argv[2];
    string dstfile = argv[3];
    int channel = stoi(argv[4]);
    float startpos = stof(argv[5]);
    float endpos = stof(argv[6]);
    string artlabel = argv[7];

    EEGStudy *rstudy;

    std::cout << "----------------------------------------------------------------------------------" << std::endl;
    std::cout << "Opening filename: " << edfsrcfile << std::endl;

    float windowLen;

    if (strcmp("none", artlabel.c_str()) == 0)
        windowLen = 0.0f;
    else
        windowLen = 1.0f;


    rstudy = new EEGStudy();
    rstudy->loadEDFfile(edfsrcfile, false);

    if (rstudy == NULL) {
        std::cout << "Error opening file - ABORTING." << std::endl;
        exit(-2);
    }

    //std::cout << "File duration (s) : " << stoi(rstudy->header->numDataRecs) * stoi(rstudy->header->recDuration) << std::endl;
    short *data;
    bool *artFlags;

    // data is a short *; getSegment should return the address to the slice of the array containing requested segment of data
    int numsamples = rstudy->getSegment(data, artFlags, artheader, channel, startpos, endpos, windowLen);

    strcpy(artheader.label, artlabel.c_str());
    //std::cout << "about to invoke copy operator..." << std::endl;
    assign(&artheader.signalMetadata, rstudy->signals[channel]);
    //std::cout << "copy complete!" << std::endl;

    std::cout << "Writing file: " << dstfile << " channel: " << channel << " numsamples: " << numsamples <<
              " startpos: " << startpos << " endpos: " << endpos << std::endl;

    FILE *fp = fopen(dstfile.c_str(), "wb");
    // first, write the header
    fwrite(&artheader, sizeof(EEGArtifactV4), 1, fp);
    //now write the data, an array of shorts
    fwrite(data, sizeof(short), numsamples, fp);
    //now write the artifact flags
    fwrite(artFlags, sizeof(bool), numsamples, fp);

    fflush(fp);
    fclose(fp);


    free(data);
    free(artFlags);

    //now let's read it back in to see if it worked
    EEGArtifactV4 test;

    fp = fopen(dstfile.c_str(), "rb");
    fread(&test, sizeof(EEGArtifactV4), 1, fp);
    short indata[numsamples];
    fread(indata, sizeof(short), numsamples, fp);
    fclose(fp);

    std::cout << "Header Data" << std::endl;
    std::cout << "Version    : " << test.version << std::endl;
    std::cout << "Channel    : " << test.channel << std::endl;
    std::cout << "Label      : " << test.label << std::endl;
    std::cout << "Numsamples : " << test.numSamples << std::endl << std::endl;
/*
	std::cout << "metadata label          : " << test.signalMetadata.label << std::endl;
	std::cout << "metadata transducerType : " << test.signalMetadata.transducerType << std::endl;
	std::cout << "metadata physDimension  : " << test.signalMetadata.physDimension << std::endl;
	std::cout << "metadata physMinimum    : " << test.signalMetadata.physMinimum << std::endl;
	std::cout << "metadata physMaximum    : " << test.signalMetadata.physMaximum << std::endl;
	std::cout << "metadata digiMinimum    : " << test.signalMetadata.digiMinimum << std::endl;
	std::cout << "metadata digiMaximum    : " << test.signalMetadata.digiMaximum << std::endl;
	std::cout << "metadata prefilter      : " << test.signalMetadata.prefilter << std::endl;
	std::cout << "metadata numSamples     : " << test.signalMetadata.numSamples << std::endl;
	std::cout << "metadata reserved       : " << test.signalMetadata.reserved << std::endl;
	for (int i=0;i<numsamples;i++)
		std::cout << "indata[" << i << "]: " << indata[i] << std::endl;
*/
    std::cout << "Success! - exiting." << std::endl;

    std::cout << "----------------------------------------------------------------------------------" << std::endl;

    if (data) {
//		free(data);
        data = NULL;
    }

    delete rstudy;
    return (0);
}
