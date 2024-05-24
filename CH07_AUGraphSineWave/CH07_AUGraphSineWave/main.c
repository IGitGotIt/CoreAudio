#include <AudioToolbox/AudioToolbox.h>

#define sineFrequency 880.0


typedef struct MySineWavePlayer
{
	AudioUnit outputUnit;
	double startingFrameCount;
    double lastSwapTime;
    int currentChannel;  // 0 for left, 1 for right
} MySineWavePlayer;

OSStatus SineWaveRenderProc(void *inRefCon,
							AudioUnitRenderActionFlags *ioActionFlags,
							const AudioTimeStamp *inTimeStamp,
							UInt32 inBusNumber,
							UInt32 inNumberFrames,
							AudioBufferList * ioData);
void CreateAndConnectOutputUnit (MySineWavePlayer *player) ;

#pragma mark - callback function -
OSStatus SineWaveRenderProc(void *inRefCon,
							AudioUnitRenderActionFlags *ioActionFlags,
							const AudioTimeStamp *inTimeStamp,
							UInt32 inBusNumber,
							UInt32 inNumberFrames,
							AudioBufferList * ioData)
{
	//	printf ("SineWaveRenderProc needs %ld frames at %f\n", inNumberFrames, CFAbsoluteTimeGetCurrent());
    MySineWavePlayer *player = (MySineWavePlayer*)inRefCon;
    double j = player->startingFrameCount;
    double cycleLength = 44100. / sineFrequency;
    int frame = 0;
    for (frame = 0; frame < inNumberFrames; ++frame){
        Float32 *data = (Float32*)ioData->mBuffers[0].mData;
        (data)[frame] = (Float32)sin (2 * M_PI * (j / cycleLength));
       // (data)[frame] = 0;
        
        data = (Float32*)ioData->mBuffers[1].mData;
        (data)[frame] = (Float32)sin (2 * M_PI * (j / cycleLength));
        (data)[frame] = 0;
        j += 1.0;
        if (j > cycleLength)
            j -= cycleLength;
    }
    player->startingFrameCount = j;
    return noErr;

//
//      double cycleLength = 44100.0 / sineFrequency; // Using player's sine frequency
//      double frameDuration = 1.0 / 44100.0; // assuming a sample rate of 44100 Hz
//      double j = player->startingFrameCount;
//      
//      // Determine if we need to swap channels
//      double currentTime = inTimeStamp->mSampleTime * frameDuration;
//      if (currentTime - player->lastSwapTime >= 5.0) { // Swap every 2 seconds
//          player->currentChannel = 1 - player->currentChannel;  // Toggle between 0 and 1
//          player->lastSwapTime = currentTime;
//      }
//
//      int16_t *audioData = (int16_t*)ioData->mBuffers[0].mData;
//
//      for (int frame = 0; frame < inNumberFrames; ++frame) {
//          // Generate sample and scale it to 16-bit signed integer range
//          int16_t sampleValue = (int16_t)(sin(2 * M_PI * (j / cycleLength)) * 32767);
//          
//          // Calculate buffer indices for interleaved data
//          int index = frame * 2;  // 2 channels per frame
//
//          if (player->currentChannel == 0) {
//              audioData[index] = sampleValue;   // Sine wave on left channel
//              audioData[index + 1] = 0;         // Mute right channel
//          } else {
//              audioData[index] = 0;             // Mute left channel
//              audioData[index + 1] = sampleValue; // Sine wave on right channel
//          }
//
//          j += 1.0;
//          if (j > cycleLength)
//              j -= cycleLength;
//      }
//
//      player->startingFrameCount = j;
//      return noErr;
//    MySineWavePlayer *player = (MySineWavePlayer*)inRefCon;
//
//       double cycleLength = 44100.0 / sineFrequency; // Using player's sine frequency
//       double frameDuration = 1.0 / 44100.0; // assuming a sample rate of 44100 Hz
//       double j = player->startingFrameCount;
//       
//       // Determine if we need to swap channels
//       double currentTime = inTimeStamp->mSampleTime * frameDuration;
//       if (currentTime - player->lastSwapTime >= 2.0) { // Swap every 2 seconds
//           player->currentChannel = 1 - player->currentChannel;  // Toggle between 0 and 1
//           player->lastSwapTime = currentTime;
//       }
//
//       int16_t *leftChannelData = (int16_t*)ioData->mBuffers[0].mData;
//       int16_t *rightChannelData = (int16_t*)ioData->mBuffers[1].mData;
//
//       for (int frame = 0; frame < inNumberFrames; ++frame) {
//           // Generate sample and scale it to 16-bit signed integer range
//           int16_t sampleValue = (int16_t)(sin(2 * M_PI * (j / cycleLength)) * 32767);
//
//           if (player->currentChannel == 0) {
//               leftChannelData[frame] =  sampleValue;  // Sine wave on left channel
//               rightChannelData[frame] = 0;           // Mute right channel
//           } else {
//               leftChannelData[frame] = 0;            // Mute left channel
//               rightChannelData[frame] = sampleValue; // Sine wave on right channel
//           }
//
//           j += 1.0;
//           if (j > cycleLength)
//               j -= cycleLength;
//       }
//
//       player->startingFrameCount = j;
//       return noErr;
}

#pragma mark - utility functions -

// generic error handler - if err is nonzero, prints error message and exits program.
static void CheckError(OSStatus error, const char *operation)
{
	if (error == noErr) return;
	
	char str[20];
	// see if it appears to be a 4-char-code
	*(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
	if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
		str[0] = str[5] = '\'';
		str[6] = '\0';
	} else
		// no, format it as an integer
		sprintf(str, "%d", (int)error);
	
	fprintf(stderr, "Error: %s (%s)\n", operation, str);
	
	exit(1);
}


void CreateAndConnectOutputUnit (MySineWavePlayer *player) {
	
	//  10.6 and later: generate description that will match out output device (speakers)
	AudioComponentDescription outputcd = {0}; // 10.6 version
	outputcd.componentType = kAudioUnitType_Output;
	outputcd.componentSubType = kAudioUnitSubType_DefaultOutput;
	outputcd.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	AudioComponent comp = AudioComponentFindNext (NULL, &outputcd);
	if (comp == NULL) {
		printf ("can't get output unit");
		exit (-1);
	}
	CheckError (AudioComponentInstanceNew(comp, &player->outputUnit),
				"Couldn't open component for outputUnit");
	
    AudioStreamBasicDescription asbd;
    memset(&asbd, 0, sizeof(asbd));
    asbd.mSampleRate = 44100; // Example sample rate
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked ;//| kLinearPCMFormatFlagIsNonInterleaved;
    asbd.mFramesPerPacket = 1;
    asbd.mChannelsPerFrame = 2; // Stereo
    asbd.mBitsPerChannel = 16;
    asbd.mBytesPerPacket = asbd.mBytesPerFrame = (asbd.mBitsPerChannel / 8) * asbd.mChannelsPerFrame;
    asbd.mBytesPerFrame = (asbd.mBitsPerChannel / 8) * asbd.mChannelsPerFrame;
    
//    AudioUnitSetProperty(player->outputUnit,
//                         kAudioUnitProperty_StreamFormat,
//                         kAudioUnitScope_Input,  // or kAudioUnitScope_Output, depending on the direction
//                         0,                      // bus number
//                         &asbd,
//                         sizeof(asbd));
    AudioUnitSetProperty(player->outputUnit,
                         kAudioUnitProperty_StreamFormat,
                         kAudioUnitScope_Output,  // or kAudioUnitScope_Output, depending on the direction
                         1,                      // bus number
                         &asbd,
                         sizeof(asbd));

	// register render callback
	AURenderCallbackStruct input;
	input.inputProc = SineWaveRenderProc;
	input.inputProcRefCon = player;
	CheckError(AudioUnitSetProperty(player->outputUnit,
									kAudioUnitProperty_SetRenderCallback, 
									kAudioUnitScope_Input,
									0,
									&input,
									sizeof(input)),
			   "AudioUnitSetProperty failed");
		
	// initialize unit
	CheckError (AudioUnitInitialize(player->outputUnit),
				"Couldn't initialize output unit");
	
}

#pragma mark main

int	main(int argc, const char *argv[])
{
 	MySineWavePlayer player = {0};
	
	// set up unit and callback
	CreateAndConnectOutputUnit(&player);
	
	// start playing
	CheckError (AudioOutputUnitStart(player.outputUnit), "Couldn't start output unit");
	
	printf ("playing\n");
	// play for 5 seconds
	sleep(50);
cleanup:
	AudioOutputUnitStop(player.outputUnit);
	AudioUnitUninitialize(player.outputUnit);
	AudioComponentInstanceDispose(player.outputUnit);
	
	return 0;
}
