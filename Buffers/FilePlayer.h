//
//  FilePlayer.h
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#ifndef __FilePlayer__
#define __FilePlayer__

#include <stdio.h>
#include <stdint.h>

class Buffer;

class FilePlayer
{
public:
	
	FilePlayer(Buffer *buffer);
	
	~FilePlayer();
	
	virtual void play();
	
	virtual void pause();
	
	virtual void stop();
	
	inline const bool &isPlaying() { return mIsPlaying; }
	
	inline const int64_t &numFramesPlayed() { return mNumFramesPlayed; }
	
	virtual void processShort(short* const buffer, const int frameCount);
	
	virtual void processFloat(float** const buffers, const int frameCount);
	
	inline Buffer* const &getBuffer() { return mBuffer; }
	
protected:
	
	bool mIsPlaying;
	
	int64_t mNumFramesPlayed;
	
	Buffer *mBuffer;
};

#endif /* defined(__FilePlayer__) */