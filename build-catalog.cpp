//
// build-catalog.cpp - construct a file catalog of EEG Snippets
// documented in jira EDF-2
// created by robabbott on 8/25/20.
//

#include <iostream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include "edftest.h"

// argv[0] - program name (e.g., "build-catalog")
// argv[1] - fqdn of starting directory (e.g., "/mnt/f5c6f0d4-e553-4895-a955-e0f62ee703f4/tuh_eeg_art_snippets/")
// output - a line entry to be concatenated to the artifact catalog
// <label>\t<electrode config>\t<fq .art filename>

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cout << "build-catalog : invalid number of parameters." << std::endl;
        std::cout << "usage:" << std::endl;
        std::cout << "build-catalog <fqdn-start>" << std::endl;
        std::cout << "\t<fqdn-start> \t the starting tld to start processing artifact snippets" << std::endl;

        std::cout << std::endl << "build-catalog - exiting (-1)." << std::endl;
        return -1;
    }

    string startDir = argv[1];

    EEGArtifactV3 snippet;

    FILE *fp = fopen(startDir.c_str(), "rb");
    fread(&snippet, sizeof(EEGArtifactV3), 1, fp);
    fclose(fp);

    std::cout << snippet.label << "\t" << snippet.signalMetadata.label << "\t" << argv[1] << std::endl; 

    return 0;
}
