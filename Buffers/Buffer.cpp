//
//  Buffer.cpp
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 20/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#include <stdlib.h>
#include "sndfile.h"
#include "Buffer.h"

SF_INFO mFileInfo = {};
SNDFILE *mFile = nullptr;

int check_sf_error(int tag, int error, SNDFILE *file);


Buffer::Buffer(const double sampleRate, const char* inFilePath)
{
	mFile = sf_open(inFilePath, SFM_READ, &mFileInfo);
	check_sf_error(0, 0, mFile);
	
	printf("Opening file:\n - filepath: %s\n - samplerate: %d\n - numchannels: %d\n - numframes: %lld\n\n",
		   inFilePath, mFileInfo.samplerate, mFileInfo.channels, mFileInfo.frames);
	
	mFloatData = new float[(getNumFrames() * getNumChannels())];
	sf_count_t framesRead = sf_read_float(mFile, (float *)mFloatData, mFileInfo.frames);
	if (framesRead != mFileInfo.frames)
		printf("Incorrect number of frames read (%lld) from file\n", framesRead);
}

Buffer::~Buffer()
{
	check_sf_error(6, sf_close(mFile), mFile);
	delete[] mFloatData;
}

const int Buffer::getNumChannels()
{
	return mFileInfo.channels;
}

const int64_t Buffer::getNumFrames()
{
	return mFileInfo.frames;
}

const double Buffer::getSampleRate()
{
	return (double)mFileInfo.samplerate;
}


int check_sf_error(int tag, int error, SNDFILE *file) {
	if (error || file == NULL) {
		printf("(#%d libsndfile error) %s\n", tag, sf_strerror(file));
		return 1;
	}
	return 0;
}