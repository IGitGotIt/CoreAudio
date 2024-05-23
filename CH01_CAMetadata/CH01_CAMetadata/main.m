#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	
    // Assuming the path is correct and the file exists at this location
    NSString *audioFilePath = @"/Users/jaideepshah/Desktop/coreAudio/Learning-Core-Audio-Book-Code-Sample/CH01_CAMetadata/CH01_CAMetadata/stereo-test.mp3";

    // Create a file URL from the path
    NSURL *audioURL = [NSURL fileURLWithPath:audioFilePath];

    // You can check if the URL was created and point to a valid file
    if ([[NSFileManager defaultManager] fileExistsAtPath:audioFilePath]) {
        NSLog(@"Audio file loaded successfully.");
    } else {
        NSLog(@"Failed to load audio file.");
    }


	NSLog (@"audioURL: %@", audioURL);
	AudioFileID audioFile;	// 4
	OSStatus theErr = noErr;	// 5
	theErr = AudioFileOpenURL((CFURLRef)audioURL, kAudioFileReadPermission, 0, &audioFile); // 6
	assert (theErr == noErr);	// 7
	UInt32 dictionarySize = 0;	// 8
	theErr = AudioFileGetPropertyInfo (audioFile, kAudioFilePropertyInfoDictionary,
									   &dictionarySize, 0); // 9
	assert (theErr == noErr);	// 10
	CFDictionaryRef dictionary;	// 11
	theErr = AudioFileGetProperty (audioFile, kAudioFilePropertyInfoDictionary,
								   &dictionarySize, &dictionary); // 12
	assert (theErr == noErr);	// 13
	NSLog (@"dictionary: %@", dictionary);	// 14
	CFRelease (dictionary);	// 15
	theErr = AudioFileClose (audioFile);	// 16
	assert (theErr == noErr);	// 17
	
    [pool drain];
    return 0;
}
