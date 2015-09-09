//
//  VarispeedPlayer.h
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#ifndef __VarispeedPlayer__
#define __VarispeedPlayer__

#include "FilePlayer.h"

class VarispeedPlayer : public FilePlayer
{
public:
	
	VarispeedPlayer(Buffer *buffer);
	
	~VarispeedPlayer();
	
	void play();
	
	void pause();
	
	void stop();
	
	inline void setRate(const float rate) { mRate = rate; }
	
	inline const float& getRate() { return mRate; }
	
	void processFloat(float** const buffers, const int frameCount);
	
protected:
	
	float mRate;
	
	float mPlayhead;
};

#endif /* defined(__VarispeedPlayer__) */