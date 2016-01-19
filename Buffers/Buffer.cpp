//
//  Buffer.cpp
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 20/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//


#include "Buffer.h"

#if __APPLE__

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>

#define checkResult(result,operation) (_checkResult((result),(operation),strrchr(__FILE__, '/')+1,__LINE__))
static inline bool _checkResult(OSStatus result, const char *operation, const char* file, int line) {
	if ( result != noErr ) {
		printf("%s:%d: %s result %d %08X %4.4s\n", file, line, operation, (int)result, (int)result, (char*)&result);
		return false;
	}
	return true;
}

static AudioStreamBasicDescription nonInterleavedFloatMonoAudioDescription();
static AudioStreamBasicDescription nonInterleavedFloatStereoAudioDescription();
static AudioBufferList *AllocateAndInitAudioBufferList(AudioStreamBasicDescription audioFormat, int frameCount);

static ExtAudioFileRef file;
static int64_t totalFrames = 0;
static int channelCount = 2;
static AudioBufferList* bufferList;;

#else

#include <stdio.h>
#include "sndfile.h"

SF_INFO fileInfo = {};
SNDFILE *file = nullptr;

int check_sf_error(int tag, int error, SNDFILE *file) {
	if (error || file == NULL) {
		printf("(#%d libsndfile error) %s\n", tag, sf_strerror(file));
		return 1;
	}
	return 0;
}

#endif


Buffer::Buffer(const double sampleRate, const char* filePath)
{
#if __APPLE__
	
	CFStringRef path = CFStringCreateWithCString(NULL, filePath, kCFStringEncodingUTF8);

	CFURLRef fileURL = CFURLCreateWithString(NULL, path, NULL);
	
	OSErr errorCode = ExtAudioFileOpenURL(fileURL, &file); // TODO: Support spaces in filepath
	assert( checkResult(errorCode, "Unable to open file. Perhaps the path contains spaces?") );
	
	SInt64 fileSize;
	UInt32 propertySize = sizeof(fileSize);
	assert( checkResult(ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileLengthFrames, &propertySize, &fileSize),
						"Error getting file length") );

	// Get file format
	AudioStreamBasicDescription fileASBD;
	propertySize = sizeof(fileASBD);
	assert( checkResult(ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileDataFormat, &propertySize, &fileASBD),
						"Error getting file format") );
	channelCount = fileASBD.mChannelsPerFrame;
	
	// Calculate file size
	AudioStreamBasicDescription destinationASBD = nonInterleavedFloatMonoAudioDescription();
	totalFrames = fileSize * (destinationASBD.mSampleRate / fileASBD.mSampleRate);

	// Set destination format
	assert( checkResult(ExtAudioFileSetProperty(file, kExtAudioFileProperty_ClientDataFormat, sizeof(destinationASBD), &destinationASBD),
						"Error setting client format") );
	
	// Read audio
	AudioBufferList* audioBufferList = AllocateAndInitAudioBufferList(destinationASBD, totalFrames);
	UInt32 readFrames = totalFrames;
	assert( checkResult(ExtAudioFileRead(file, &readFrames, audioBufferList), "Error reading file" ) );
	
	floatData = new float[getNumFrames() * getNumChannels()];
	memcpy(floatData, audioBufferList->mBuffers[0].mData, audioBufferList->mBuffers[0].mDataByteSize);
	
#else
	
	file = sf_open(inFilePath, SFM_READ, &fileInfo);
	check_sf_error(0, 0, file);
	
	floatData = new float[getNumFrames() * getNumChannels()];
	
	sf_count_t framesRead = sf_read_float(file, (float *)floatData, fileInfo.frames);
	if (framesRead != fileInfo.frames)
		printf("Incorrect number of frames read (%lld) from file\n", framesRead);
	
#endif
	
	printf("Opening file:\n - filepath: %s\n - samplerate: %f\n - numchannels: %d\n - numframes: %lld\n\n",
		   filePath,
		   getSampleRate(),
		   getNumChannels(),
		   getNumFrames());
}

Buffer::~Buffer()
{
#if __APPLE__
	ExtAudioFileDispose(file);
#else
	check_sf_error(6, sf_close(file), file);
#endif
	
	delete[] floatData;
}

const int Buffer::getNumChannels()
{
#if __APPLE__
	return channelCount;
#else
	return fileInfo.channels;
#endif
}

const int64_t Buffer::getNumFrames()
{
#if __APPLE__
	return totalFrames;
#else
	return fileInfo.frames;
#endif
}

const double Buffer::getSampleRate()
{
#if __APPLE__
	return 44100.0;
#else
	return (double)fileInfo.samplerate;
#endif
}


#if __APPLE__

/**
 * Floating-point PCM, stereo, non-interleaved format at 44.1kHz
 */
static AudioStreamBasicDescription nonInterleavedFloatMonoAudioDescription()
{
	AudioStreamBasicDescription audioDescription;
	memset(&audioDescription, 0, sizeof(audioDescription));
	audioDescription.mFormatID          = kAudioFormatLinearPCM;
	audioDescription.mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
	audioDescription.mChannelsPerFrame  = 1;
	audioDescription.mBytesPerPacket    = sizeof(float);
	audioDescription.mFramesPerPacket   = 1;
	audioDescription.mBytesPerFrame     = sizeof(float);
	audioDescription.mBitsPerChannel    = 8 * sizeof(float);
	audioDescription.mSampleRate        = 44100.0;
	return audioDescription;
}

/**
 * Floating-point PCM, stereo, non-interleaved format at 44.1kHz
 */
static AudioStreamBasicDescription nonInterleavedFloatStereoAudioDescription()
{
	AudioStreamBasicDescription audioDescription;
	memset(&audioDescription, 0, sizeof(audioDescription));
	audioDescription.mFormatID          = kAudioFormatLinearPCM;
	audioDescription.mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
	audioDescription.mChannelsPerFrame  = 2;
	audioDescription.mBytesPerPacket    = sizeof(float);
	audioDescription.mFramesPerPacket   = 1;
	audioDescription.mBytesPerFrame     = sizeof(float);
	audioDescription.mBitsPerChannel    = 8 * sizeof(float);
	audioDescription.mSampleRate        = 44100.0;
	return audioDescription;
}

/**
 *  Allocates an AudiobufferList for specified ASBD and framecount
 */
static AudioBufferList *AllocateAndInitAudioBufferList(AudioStreamBasicDescription audioFormat, int frameCount)
{
	int numberOfBuffers = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? audioFormat.mChannelsPerFrame : 1;
	int channelsPerBuffer = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? 1 : audioFormat.mChannelsPerFrame;
	int bytesPerBuffer = audioFormat.mBytesPerFrame * frameCount;
	
	AudioBufferList *audio = (AudioBufferList *)malloc(sizeof(AudioBufferList) + (numberOfBuffers-1)*sizeof(AudioBuffer));
	if ( !audio ) return NULL;
	
	audio->mNumberBuffers = numberOfBuffers;
	for ( int i=0; i<numberOfBuffers; i++ )
	{
		if ( bytesPerBuffer > 0 )
		{
			audio->mBuffers[i].mData = calloc(bytesPerBuffer, 1);
			if ( !audio->mBuffers[i].mData )
			{
				for ( int j=0; j<i; j++ ) free(audio->mBuffers[j].mData);
				free(audio);
				return NULL;
			}
		}
		else
		{
			audio->mBuffers[i].mData = NULL;
		}
		audio->mBuffers[i].mDataByteSize = bytesPerBuffer;
		audio->mBuffers[i].mNumberChannels = channelsPerBuffer;
	}
	return audio;
}

#endif

