//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#include <string>
#include "assert.h"
#include "Buffer.h"
#include "FilePlayer.h"


FilePlayer::FilePlayer(const char* filePath, double sampleRate)
:	buffer(new Buffer(sampleRate, filePath)),
	isPlaying(false),
	framesPlayed(0)
{
	
}

FilePlayer::~FilePlayer()
{
}

void FilePlayer::play()
{
	isPlaying = true;
}

void FilePlayer::pause()
{
	isPlaying = false;
}

void FilePlayer::stop()
{
	isPlaying = false;
	framesPlayed = 0;
}

void FilePlayer::processShort(int16_t* const buffer, const int frameCount)
{
	float leftBuffer[frameCount], rightBuffer[frameCount];
	float *buffers[2] = {leftBuffer, rightBuffer};
	processFloat(buffers, frameCount);
	for (int i = 0; i < frameCount; i++)
	{
		buffer[i*2] = (int16_t)(leftBuffer[i] * 32767.f);
		buffer[i*2+1] = (int16_t)(rightBuffer[i] * 32767.f);
	}
}

void FilePlayer::processFloat(float** const buffers, const int frameCount)
{
	if ( !getIsPlaying() ) return;
	
	Buffer* buffer = getBuffer();
	
	int64_t index = getFramesPlayed();
	int64_t totalFrames = buffer->getNumFrames();
	int64_t framesLeft	= totalFrames - index;
	int64_t framesToRead = (framesLeft < frameCount && !getIsLooping()) ? framesLeft : frameCount;

	for ( int64_t i = 0; i < framesToRead; i++ )
	{
		int64_t readIndex = (index + i) % totalFrames;
		assert(readIndex <= totalFrames);
		buffers[0][i] = buffers[1][i] = buffer->getFloatData()[readIndex];
	}
	
	framesPlayed += framesToRead;
	
	if ( getFramesPlayed() >= totalFrames ) // Pause or loop on EOF
	{
		if ( getIsLooping() ) framesPlayed -= totalFrames;
		else pause();
	}
}
