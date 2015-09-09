//
//  ViewController.m
//  VarispeedPlayer
//
//  Created by Ragnar Hrafnkelsson on 15/08/2015.
//  Copyright (c) 2015 Reactify. All rights reserved.
//

#include "portaudio.h"
#include "Buffer.h"
#include "FilePlayer.h"
#include "VarispeedPlayer.h"
#import "ViewController.h"

// UserData struct to be passed to the audio callback
typedef struct
{
	VarispeedPlayer *filePlayer;
	unsigned long fileNumFrames;
	unsigned long fileIndex;
} UserData;


int check_pa_error(int tag, PaError error);

// Main Audio Processing Callback
static int paCallback(	const void *input,
						void *output,
						unsigned long frameCount,
						const PaStreamCallbackTimeInfo* timeInfo,
						PaStreamCallbackFlags statusFlags,
						void *userData)
{
	UserData *data = (UserData *) userData;
	short *out = (short *)output;
	memset(out, 0, frameCount * sizeof(short));
	
	data->filePlayer->processShort(out, (int)frameCount);
	
	return paContinue;
}


@interface ViewController()
{
	PaStream *stream;
	UserData data;
}
@end

@implementation ViewController

- (void)viewDidLoad
{
	[super viewDidLoad];
	[self start];
}

- (void)viewDidAppear
{
	[super viewDidAppear];
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
		data.filePlayer->setRate(1.f);
		data.filePlayer->play();
	});
}

- (void)viewWillDisappear
{
	[super viewWillDisappear];
	[self stop];
}

- (IBAction)onSliderValueChanged:(NSSlider *)sender
{
	NSLog(@"Slider value: %f", sender.floatValue);
	data.filePlayer->setRate(sender.floatValue);
}

- (int)start
{
	NSURL *fileURL = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"test" ofType:@"wav"]];
	const char *inFilePath = fileURL.path.UTF8String;
	
	const double sampleRate = 44100.0;// (double) info.samplerate;
	const unsigned long blockSize = 256;
	
	Buffer *fileBuffer = new Buffer(sampleRate, inFilePath);
	data.filePlayer = new VarispeedPlayer(fileBuffer);
	
	// Setup PortAudio
	if (check_pa_error(1, Pa_Initialize())) return 1;
	
	// Opening stream
	if (check_pa_error(2,
					   Pa_OpenDefaultStream(&stream, 0, 2,
											paInt16, sampleRate, blockSize, paCallback, &data))) {
						   return 1;
					   }
	
	// Start Processing
	if (check_pa_error(3, Pa_StartStream(stream))) return 1;
	
	return 0;
}

- (int)stop
{
	// Stop Processing
	if (check_pa_error(5, Pa_CloseStream(stream))) return 1;
	
	// Teardown
	
	delete data.filePlayer;
	
	if (check_pa_error(7, Pa_Terminate())) return 1;
	
	printf("Success\n");
	return 0;
}


int check_pa_error(int tag, PaError error) {
	if (error != paNoError) {
		printf("(#%d portaudio error) %s\n", tag, Pa_GetErrorText(error));
		return 1;
	}
	return 0;
}

@end
