//////////////////////////////////////////////////////////////////////////
//
//
//					InputFileMPEG
//
//
//////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "FXProcs.h"

class InputFileMPEG
{
private:
	char *video_packet_buffer;
	char *audio_packet_buffer;

	unsigned int ScanCpos;
	bool	NextStartCode();

	int 	Read(void *, int);
	int 	_Read();
        //Bufor
        char *fileStartBuf;
        unsigned int bufSize;

        //MPEGAudioParser
	unsigned long lFirstAudioHeader;
	int hAudioState, audioSkip;
	unsigned long AudioHeader;
        //MPEGVideoParser
	unsigned char videoBuf[72+64];
	int videoIdx, videoBytes;
	long videoHeader;
	bool fFoundSequenceStart;
public:
	InputFileMPEG();
	~InputFileMPEG();

	int width, height;
	long frame_rate;
	bool fInterleaved;
	int layer;
	int audBitrate;
	int samprate;
        int audChannels;

        void ReinitParser(char *nFileStartBuf,unsigned int nBufSize);
        void ParseAudio(const void *pData, int len);
	void ParseVideo(const void *, int);
	void ParseMPEGFile();
};


