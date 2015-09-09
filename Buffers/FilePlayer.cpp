//
//  FilePlayer.cpp
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#include <stdlib.h>
#include <string>
#include "Buffer.h"
#include "FilePlayer.h"


FilePlayer::FilePlayer(Buffer *buffer) : mBuffer(buffer), mIsPlaying(false), mNumFramesPlayed(0) {}

FilePlayer::~FilePlayer()
{
	delete mBuffer;
}

void FilePlayer::play()
{
	mIsPlaying = true;
}

void FilePlayer::pause()
{
	mIsPlaying = false;
}

void FilePlayer::stop()
{
	mIsPlaying = false;
	mNumFramesPlayed = 0;
}

void FilePlayer::processShort(short* const buffer, const int frameCount)
{
	float leftBuffer[frameCount], rightBuffer[frameCount];
	memset(leftBuffer, 0, frameCount * sizeof(float));
	memset(rightBuffer, 0, frameCount * sizeof(float));
	float *buffers[2] = {leftBuffer, rightBuffer};
	processFloat(buffers, frameCount);
	for (int i = 0; i < frameCount; i++)
	{
		buffer[i*2] = (short)(leftBuffer[i] * 32767.f);
		buffer[i*2+1] = (short)(rightBuffer[i] * 32767.f);
	}
}

void FilePlayer::processFloat(float** const buffers, const int frameCount)
{
	if ( !isPlaying() ) return;
	
	int64_t numFramesLeft = getBuffer()->getNumFrames() - numFramesPlayed();
	int64_t numFramesToRead = (numFramesLeft < frameCount) ? numFramesLeft : frameCount;
	
	unsigned long index = numFramesPlayed();
	memcpy(buffers[0], getBuffer()->getFloatData()+index, numFramesToRead * sizeof(float));
	memcpy(buffers[1], getBuffer()->getFloatData()+index, numFramesToRead * sizeof(float));
	mNumFramesPlayed += numFramesToRead;
	
	// Pause on EOF
	if (numFramesPlayed() >= getBuffer()->getNumFrames())
		pause();
}
