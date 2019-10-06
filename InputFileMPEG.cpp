//////////////////////////////////////////////////////////////////////////
//
//
//							InputFileMPEG
//
//
//////////////////////////////////////////////////////////////////////////

#include "InputFileMPEG.h"
#include <vcl.h>

InputFileMPEG::InputFileMPEG() {
	// clear variables

	video_packet_buffer = NULL;
	audio_packet_buffer = NULL;

        ReinitParser(NULL,0);

}

void InputFileMPEG::ReinitParser(char *nFileStartBuf,unsigned int nBufSize)
{
        fileStartBuf=nFileStartBuf;
        bufSize=nBufSize;
	fInterleaved = false;

        //MPEGAudioParser
	lFirstAudioHeader = 0;
	AudioHeader = 0;
	hAudioState = 0;
	audioSkip = 0;
	layer = 0;
	audBitrate = 0;
	samprate = 0;
        audChannels = 0;
        //MPEGVideoParser
	videoHeader = 0xFFFFFFFF;
	fFoundSequenceStart = false;
        //Parametry strumieni
		this->width = 0;
		this->height = 0;
		this->frame_rate = 0;

	videoIdx = videoBytes = 0;

	delete video_packet_buffer;
	delete audio_packet_buffer;

    // allocate packet buffer
	video_packet_buffer = new char[bufSize];
	audio_packet_buffer = new char[bufSize];
}

#define VIDPKT_TYPE_SEQUENCE_START		(0xb3)
#define	VIDPKT_TYPE_SEQUENCE_END		(0xb7)
#define VIDPKT_TYPE_GROUP_START			(0xb8)
#define VIDPKT_TYPE_PICTURE_START		(0x00)
#define VIDPKT_TYPE_SLICE_START_MIN		(0x01)
#define	VIDPKT_TYPE_SLICE_START_MAX		(0xaf)
#define VIDPKT_TYPE_EXT_START			(0xb5)
#define VIDPKT_TYPE_USER_START			(0xb2)

void InputFileMPEG::ParseMPEGFile()
{
	bool finished = false;

	enum {
		RESERVED_STREAM		= 0xbc,
		PRIVATE_STREAM1		= 0xbd,
		PADDING_STREAM		= 0xbe,
		PRIVATE_STREAM2		= 0xbf,
	};


	// Begin file parsing!  This is a royal bitch!

	ScanCpos = 0;

	try {

		bool first_packet = true;
		bool end_of_file = false;

		// seek to first pack code

		{
			char ch[3];
			int scan_count = 256;

			Read(ch, 3);

			while(scan_count > 0) {
                                if (ch[0]==0 && ch[1]==0 && ch[2]==1) {
					// We want reads to be aligned.

					ScanCpos =(256 + 3 - scan_count);

					break;
				}

				ch[0] = ch[1];
				ch[1] = ch[2];
				Read(ch+2, 1);

				--scan_count;

				if (!scan_count) return;
			}
		}

		try {
			do {
				int c;
				int stream_id, pack_length;

				if (first_packet) {
					{
						c = _Read();

						fInterleaved = (c==0xBA);

						if (!fInterleaved) {

							unsigned char buf[4];

							buf[0] = buf[1] = 0;
							buf[2] = 1;
							buf[3] = c;

							ParseVideo(buf, 4);
						}
					}

					first_packet = false;
				} else if (fInterleaved)
					c=_Read();
				else
					c = 0xe0;

				switch(c) {

//					One for audio and for video?

					case VIDPKT_TYPE_SEQUENCE_END:
					case 0xb9:		// ISO 11172 end code
						break;

					case 0xba:		// new pack
						if ((_Read() & 0xf0) != 0x20) return;//throw MyError("%s: pack synchronization error", szME);
						ScanCpos+=7;
						break;

					case 0xbb:		// system header
						ScanCpos+=8;
						while((c=_Read()) & 0x80)
							ScanCpos+=2;
						ScanCpos-=1;

						break;

					default:
						if (c < 0xc0 || c>=0xf0)
							break;

						if (fInterleaved)
                                                 {
							stream_id = c;
							pack_length = _Read()<<8;
							pack_length += _Read();

							if (stream_id != PRIVATE_STREAM2)
                                                         {
								--pack_length;

								while((c=_Read()) == 0xff) {
									--pack_length;
								}

								if ((c>>6) == 1) {	// 01
									pack_length-=2;
									ScanCpos+=1;			// skip one byte
									c=_Read();
								}
								if ((c>>4) == 2) {	// 0010
									pack_length -= 4;
									ScanCpos+=4;
								} else if ((c>>4) == 3) {	// 0011
									pack_length -= 9;
									ScanCpos+=9;
								} else if (c != 0x0f)
									return;//throw MyError("%s: packet sync error on packet stream (%I64x)", szME, tagpos);
							 }
						 } else //if (fInterleaved)
                                                 {
							stream_id = 0xe0;
							pack_length = 65536; //VIDEO_PACKET_BUFFER_SIZE;
						 }

						// check packet type

						if ((0xe0 & stream_id) == 0xc0) {			// audio packet

							Read(audio_packet_buffer, pack_length);
							ParseAudio(audio_packet_buffer, pack_length);
							pack_length = 0;

						} else if ((0xf0 & stream_id) == 0xe0) {	// video packet

							int actual = Read(video_packet_buffer, pack_length);

							if (!fInterleaved && actual < pack_length)
								end_of_file = true;

							ParseVideo(video_packet_buffer, actual);
							pack_length = 0;
						}

						if (pack_length)
								ScanCpos+=pack_length;
						break;
				}
                          if (!finished)
                            {
                            if (fInterleaved)
                                finished=!NextStartCode();
                              else
                                finished=end_of_file;
                            };
			} while(!finished);// && (fInterleaved ? NextStartCode() : !end_of_file)
		}
                __except(EXCEPTION_EXECUTE_HANDLER)
                 {

		 }

		// We're done scanning the file.  Finish off any ending packets we may have.

		//static const unsigned char finish_tag[]={ 0, 0, 1, 0xff };

		//ParseVideo(finish_tag, 4);


		// If we are accepting partial streams, then cut off the last video frame, as it may be incomplete.
		// The audio parser checks for the entire frame to arrive, so we don't need to trim the audio.

	}
       __except(EXCEPTION_EXECUTE_HANDLER)
       {
	}

}

InputFileMPEG::~InputFileMPEG()
{
	delete video_packet_buffer;
	delete audio_packet_buffer;
}

bool InputFileMPEG::NextStartCode()
{
	int c;

	while(EOF!=(c=_Read())) {
		if (!c) {	// 00
			if (EOF==(c=_Read())) return false;

			if (!c) {	// 00 00
				do {
					if (EOF==(c=_Read())) return false;
				} while(!c);

				if (c==1)	// (00 00 ...) 00 00 01 xx
					return true;
			}
		}
	}

	return false;
}

int InputFileMPEG::_Read()
{
	char c;
	if (!Read(&c, 1))
		return EOF;
	return (unsigned char)c;
}

int InputFileMPEG::Read(void *buffer, int bytes)
{
  if (ScanCpos>=bufSize)
      return 0;
  int ToRead=min((unsigned int)bytes,bufSize-ScanCpos);
  memcpy(buffer, fileStartBuf+ScanCpos, ToRead);
  ScanCpos+=ToRead;
  return ToRead;
}

void InputFileMPEG::ParseAudio(const void *pData, int len)
{
    static const int bitrate[3][15] =
        {
          {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
          {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384},
          {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320}
        };
    static const long samp_freq[4] = {44100, 48000, 32000, 0};

	unsigned char *src = (unsigned char *)pData;
	while(len>0) {
		if (audioSkip) {
			int tc = audioSkip;

			if (tc > len)
				tc = len;

			len -= tc;
			audioSkip -= tc;
			src += tc;

			// Audio frame finished?
			continue;
		}

		// Collect header bytes.

		++hAudioState;
		AudioHeader = (AudioHeader>>8) | ((long)*src++ << 24);
		--len;

        //audio_header(long hdr)
	// 0000F0FF 12 bits	sync mark
	//
	// 00000800  1 bit	version
	// 00000600  2 bits	layer (3 = layer I, 2 = layer II, 1 = layer III)
	// 00000100  1 bit	error protection (0 = enabled)
	//
	// 00F00000  4 bits	bitrate_index
	// 000C0000  2 bits	sampling_freq
	// 00020000  1 bit	padding
	// 00010000  1 bit	extension
	//
	// C0000000  2 bits	mode (0=stereo, 1=joint stereo, 2=dual channel, 3=mono)
	// 30000000  2 bits	mode_ext
	// 08000000  1 bit	copyright
	// 04000000  1 bit	original
	// 03000000  2 bits	emphasis
		if (hAudioState>=4)
                 {
			if (lFirstAudioHeader && ((AudioHeader ^ lFirstAudioHeader) & 0x08FC0E00))
                          {
				continue;
			  }

			// Okay, we like the header.

			hAudioState = 0;

			// Must be a frame start.

			if (!lFirstAudioHeader)
				lFirstAudioHeader = AudioHeader;

			long lFrameLen;

			// Layer I works in units of 4 bytes.  Layer II and III have byte granularity.
			// Both may or may not have one extra unit depending on the padding bit.

			layer		= ((AudioHeader>>9)&3)^3;
			int bitrateidx		= (AudioHeader>>20)&15;
			samprate		= samp_freq[(AudioHeader>>18)&3];
			bool padding		= !!(AudioHeader & 0x00020000);
                        audBitrate=bitrate[layer][bitrateidx];
                        audChannels=(AudioHeader>>30)&3;

			if (!layer) {	// layer I
				lFrameLen = 4*((bitrate[layer][bitrateidx]*12000) / samprate);
				if (padding) lFrameLen+=4;
			} else {													// layer II, III
				lFrameLen = (bitrate[layer][bitrateidx] * 144000) / samprate;
				if (padding) ++lFrameLen;
			}

			// Now skip the remainder of the sample.

			audioSkip = lFrameLen-4;
		 }
	}

}


void InputFileMPEG::ParseVideo(const void *pData, int len)
{
	static const long frame_speeds[16]=
              {
		0,			// 0
		23976,		// 1 (23.976 fps) - FILM
		24000,		// 2 (24.000 fps)
		25000,		// 3 (25.000 fps) - PAL
		29970,		// 4 (29.970 fps) - NTSC
		30000,		// 5 (30.000 fps)
		50000,		// 6 (50.000 fps) - PAL noninterlaced
		59940,		// 7 (59.940 fps) - NTSC noninterlaced
		60000,		// 8 (60.000 fps)
		0,			// 9
		0,			// 10
		0,			// 11
		0,			// 12
		0,			// 13
		0,			// 14
		0			// 15
	      };

	unsigned char *src = (unsigned char *)pData;

	while(len>0) {
		if (videoIdx<videoBytes)
                {
			int tc = videoBytes - videoIdx;

			if (tc > len)
				tc = len;

			memcpy(videoBuf+videoIdx, src, tc);

			len -= tc;
			videoIdx += tc;
			src += tc;

			// Finished?

			if (videoIdx>=videoBytes)
                        {
				switch(videoHeader)
                                {
					case VIDPKT_TYPE_PICTURE_START:
						videoHeader = 0xFFFFFFFF;
						break;

					case VIDPKT_TYPE_SEQUENCE_START:
						//	12 bits: width
						//	12 bits: height
						//	 4 bits: aspect ratio
						//	 4 bits: picture rate
						//	18 bits: bitrate
						//	 1 bit : ?
						//	10 bits: VBV buffer
						//	 1 bit : const_param
						//	 1 bit : intramatrix present
						//[256 bits: intramatrix]
						//	 1 bit : nonintramatrix present
						//[256 bits: nonintramatrix]
						if (videoBytes == 8)
                                                {
							width	= (videoBuf[0]<<4) + (videoBuf[1]>>4);
							height	= ((videoBuf[1]<<8)&0xf00) + videoBuf[2];
							frame_rate = frame_speeds[(unsigned char)videoBuf[3] & 15];

							if (videoBuf[7]&2)
                                                        {		// Intramatrix present
								videoBytes = 72;	// can't decide yet
								break;
							} else if (videoBuf[7]&1)
                                                        {	// Nonintramatrix present
								videoBytes = 72;
								break;
							}
						} else if (videoBytes == 72)
                                                {

							if (videoBuf[7]&2)
                                                        {
								if (videoBuf[71]&1)
                                                                 {
									videoBytes = 72+64;		// both matrices
									break;
								 }
							};
						};

						// Initialize MPEG-1 video decoder.

						videoHeader = 0xFFFFFFFF;
						break;
				}
			}
			continue;
		}

		// Look for a valid MPEG-1 header

		videoHeader = (videoHeader<<8) + *src++;
		--len;

		if ((videoHeader&0xffffff00) == 0x00000100) {
			videoHeader &= 0xff;
			switch(videoHeader) {
			case VIDPKT_TYPE_SEQUENCE_START:
				if (fFoundSequenceStart) break;
				fFoundSequenceStart = true;
				videoBytes = 8;
				videoIdx = 0;
				break;

			case VIDPKT_TYPE_PICTURE_START:
				videoIdx = 0;
				videoBytes = 2;
				break;

			case VIDPKT_TYPE_EXT_START:
				return;//throw MyError("VirtualDub cannot decode MPEG-2 video streams.");

			default:
				videoHeader = 0xFFFFFFFF;
			}
		}
	}

}

