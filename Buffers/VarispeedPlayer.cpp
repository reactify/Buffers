//
//  VarispeedPlayer.cpp
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#include <mutex>
#include "Buffer.h"
#include "Heavy_Playhead.h"
#include "Heavy_TableReader.h"
#include "VarispeedPlayer.h"

static const int HEAVY_BUFFER_SIZE = 8192;

std::mutex mutex;
long long numFramesCopied = 0;
Hv_Playhead *hvPlayhead = NULL;
Hv_TableReader *hvTableReader = NULL;


HvTable* heavyTable()
{
	return hv_getTableForName(hvTableReader, "table");
}

void copyPreviousBuffer(Buffer *fromBuffer, int startIndex, int framesToCopy)
{
	std::lock_guard<std::mutex> guard(mutex);
	HvTable *table = heavyTable();
	float *tableBuffer = hv_table_getBuffer(table);
	numFramesCopied -= (framesToCopy * 2);
	if (numFramesCopied < 0)
	{
		numFramesCopied = 0;
		memset(tableBuffer, 0, hv_table_getLength(table) * sizeof(float));
		return;
	}
	
	memcpy(tableBuffer+startIndex, fromBuffer->getFloatData() + numFramesCopied, (framesToCopy + 2) * sizeof(float)); // + 2 for interpolation
	numFramesCopied += framesToCopy;
}

void copyNextBuffer(Buffer *fromBuffer, int startIndex, int framesToCopy)
{
	std::lock_guard<std::mutex> guard(mutex);
	HvTable *table = heavyTable();
	float *tableBuffer = hv_table_getBuffer(table);
	memcpy(tableBuffer+startIndex, fromBuffer->getFloatData() + numFramesCopied, (framesToCopy + 2) * sizeof(float)); // + 2 for interpolation
	numFramesCopied += framesToCopy;
}


VarispeedPlayer::VarispeedPlayer(Buffer *buffer) : FilePlayer(buffer), mRate(1.f), mPlayhead(0.f)
{
	hvPlayhead = hv_Playhead_new(buffer->getSampleRate());
	printf("Instantiated heavy context %s:\n - numInputChannels: %d\n - numOutputChannels: %d\n\n",
		   hv_getName(hvPlayhead),
		   hv_getNumInputChannels(hvPlayhead),
		   hv_getNumOutputChannels(hvPlayhead));
	
	// TODO: Check number of channels and create varispeed player for each
	hvTableReader = hv_TableReader_new(buffer->getSampleRate());
	printf("Instantiated heavy context %s: \n - numInputChannels: %d\n - numOutputChannels: %d\n\n",
		   hv_getName(hvTableReader),
		   hv_getNumInputChannels(hvTableReader),
		   hv_getNumOutputChannels(hvTableReader));
	
	hv_vscheduleMessageForReceiver(hvPlayhead, "SetBufferSize", 0.0, "f", (float)HEAVY_BUFFER_SIZE);
	hv_vscheduleMessageForReceiver(hvTableReader, "SetBufferSize", 0.0, "f", (float)HEAVY_BUFFER_SIZE);
	
	pause();
}

VarispeedPlayer::~VarispeedPlayer()
{
	hv_Playhead_free(hvPlayhead);
	hv_TableReader_free(hvTableReader);
}

void VarispeedPlayer::play()
{
	FilePlayer::play();
	hv_vscheduleMessageForReceiver(hvPlayhead, "Speed", 0.0, "f", mRate);
}

void VarispeedPlayer::pause()
{
	FilePlayer::pause();
	hv_vscheduleMessageForReceiver(hvPlayhead, "Speed", 0.0, "f", 0.f);
}

void VarispeedPlayer::stop()
{
	pause();
	FilePlayer::stop();
}

void VarispeedPlayer::processFloat(float** const buffers, const int frameCount)
{
	if ( !isPlaying() ) return;
	
	// Process Playhead
	static float previousRate = 1.f;
	float currentRate = getRate();
	if (currentRate != previousRate)
	{
		hv_vscheduleMessageForReceiver(hvPlayhead, "Speed", 0.0, "f", currentRate); // Set playback rate
		previousRate = getRate();
	}
	float playHeadBuffer[frameCount * hv_getNumOutputChannels(hvTableReader)];
	hv_Playhead_process_inline(hvPlayhead, NULL, playHeadBuffer, (int)frameCount);
	float newPlayheadPos = playHeadBuffer[frameCount-1];
	
	// Copy from file buffer if needed
	if (currentRate >= 0.f)
	{
		static bool hasWrapped = false;
		if (playHeadBuffer[0] == 0.f ||  newPlayheadPos < mPlayhead /*wrap*/)
		{
			copyNextBuffer(getBuffer(), 0, HEAVY_BUFFER_SIZE/2);  // Buffer *fromBuffer, HvTable *table, int startIndex, int framesToCopy)
			hasWrapped = true;
		}
		else if (newPlayheadPos > 0.5 && hasWrapped)
		{
			copyNextBuffer(getBuffer(), HEAVY_BUFFER_SIZE/2, HEAVY_BUFFER_SIZE/2);
			hasWrapped = false;
		}
	}
	else
	{
		static bool hasWrapped = false;
		if (playHeadBuffer[0] == 0.f || newPlayheadPos > mPlayhead /*wrap*/)
		{
			copyPreviousBuffer(getBuffer(), HEAVY_BUFFER_SIZE/2, HEAVY_BUFFER_SIZE/2);
			hasWrapped = true;
		}
		else if (newPlayheadPos < 0.5 && hasWrapped)
		{
			copyPreviousBuffer(getBuffer(), 0, HEAVY_BUFFER_SIZE/2);
			hasWrapped = false;
		}
	}
	
	// Read from Table
	hv_TableReader_process_inline(hvTableReader, playHeadBuffer, buffers[0], (int)frameCount);
	memcpy(buffers[1], buffers[0], frameCount * sizeof(float));
	
	// Update playback state
	mPlayhead = newPlayheadPos;
	mNumFramesPlayed += (long long)(currentRate * (float)frameCount);
	if (numFramesPlayed() < 0) mNumFramesPlayed = 0;
	
	// Pause on EOF
	if (numFramesPlayed() >= getBuffer()->getNumFrames())
		pause();
}