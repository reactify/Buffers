//
//  Buffer.h
//  Buffers
//
//  Created by Ragnar Hrafnkelsson on 20/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#ifndef __Buffer__
#define __Buffer__

#include <stdio.h>
#include <stdint.h>

class Buffer
{
public:
	
	Buffer(const double sampleRate, const char* inFilePath);
	
	~Buffer();
	
	const int getNumChannels();
	
	const int64_t getNumFrames();
	
	const double getSampleRate();
	
	const float* const &getFloatData() { return mFloatData; }
	
private:
	
	const float *mFloatData;
};

#endif /* defined(__Buffer__) */
