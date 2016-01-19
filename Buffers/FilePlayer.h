//
//  FilePlayer.h
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#ifndef __FilePlayer__
#define __FilePlayer__

#include "Buffer.h"

class FilePlayer
{
public:
	FilePlayer(const char* filePath, double sampleRate);
	
	~FilePlayer();
	
	virtual void play();
	
	virtual void pause();
	
	virtual void stop();
	
	void setLoop(bool loop) { isLooping = loop; }
	
	const bool& getIsLooping() { return isLooping; }
	
	/**
	 *  Process interleaved short buffer
	 *
	 *  @param buffer     Interleaved buffer of shorts
	 *  @param frameCount Number of frames per buffer
	 */
	virtual void processShort(short* const buffer, const int frameCount);
	
	/**
	 *  Process an array of float buffers. It is the user's responsibility
	 *	to provide the correct number of buffers to match file channels
	 *
	 *  @param buffers    Array of float buffers
	 *  @param frameCount Number of frames per buffer
	 */
	virtual void processFloat(float** const buffers, const int frameCount);
	
	Buffer* const getBuffer() { return buffer.get(); }
	
	const bool& getIsPlaying() { return isPlaying; }
	
	const int64_t& getFramesPlayed() { return framesPlayed; }
	
protected:
	bool isPlaying;
	
	bool isLooping;
	
	int64_t framesPlayed;
	
	std::auto_ptr<Buffer> buffer;
};

#endif /* defined(__FilePlayer__) */
