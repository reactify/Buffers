//
//  Buffer.cpp
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 20/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#include "sndfile.h"
#include "Buffer.h"

SF_INFO fileInfo = {};
SNDFILE *file = nullptr;

int check_sf_error(int tag, int error, SNDFILE *file) {
	if (error || file == NULL) {
		printf("(#%d libsndfile error) %s\n", tag, sf_strerror(file));
		return 1;
	}
	return 0;
}


Buffer::Buffer(const double sampleRate, const char* inFilePath)
{
	file = sf_open(inFilePath, SFM_READ, &fileInfo);
	check_sf_error(0, 0, file);
	
	printf("Opening file:\n - filepath: %s\n - samplerate: %d\n - numchannels: %d\n - numframes: %lld\n\n",
		   inFilePath,
		   fileInfo.samplerate,
		   fileInfo.channels,
		   fileInfo.frames);
	
	floatData = new float[(getNumFrames() * getNumChannels())];
	
	sf_count_t framesRead = sf_read_float(file, (float *)floatData, fileInfo.frames);
	if (framesRead != fileInfo.frames)
		printf("Incorrect number of frames read (%lld) from file\n", framesRead);
}

Buffer::~Buffer()
{
	check_sf_error(6, sf_close(file), file);
	delete[] floatData;
}

const int Buffer::getNumChannels()
{
	return fileInfo.channels;
}

const int64_t Buffer::getNumFrames()
{
	return fileInfo.frames;
}

const double Buffer::getSampleRate()
{
	return (double)fileInfo.samplerate;
}
