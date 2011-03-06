/*
 *  movie.cpp
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2011-02-27.
 *  Copyright 2011 SLUDGE Developers. All rights reserved.
 *
 */

#include <SDL/SDL.h>
#include <math.h>


#include "mkvreader.hpp"
#include "mkvparser.hpp"

#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#define interface (&vpx_codec_vp8_dx_algo)

#include "newfatal.h"
#include "timing.h"
#include "graphics.h"
#include "movie.h"
#include "shaders.h"

#include "sound.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include "ogg/ogg.h"
#include "vorbis_os.h"

#include "AL/alure.h"


// in main.c
int checkInput();
extern int weAreDoneSoQuit;

// Sludger.cpp
bool handleInput ();

// sound_openal.cpp
void playStream (int a, bool isMOD, bool loopy);
int initMovieSound(int f, ALenum format, int audioChannels, ALuint samplerate, 
				   ALuint (*callback)(void *userdata, ALubyte *data, ALuint bytes));

/*
 movieIsPlaying tracks the state of movie playing
 0 = no movie
 1 = movie is played
 2 = paused
*/
int movieIsPlaying = 0;

float movieAspect = 1.6;

static void die_codec(vpx_codec_ctx_t *ctx, const char *s) {            
    //const char *detail = vpx_codec_error_detail(ctx);                         
    printf("%s: %s\n", s, vpx_codec_error(ctx));        
	fatal (s, vpx_codec_error(ctx));
 //   if(detail)                                                                
   //     printf("    %s\n",detail);                                            
}                                                                             


void setMovieViewport()
{
	float realAspect = (float) realWinWidth / realWinHeight;
	
	int vpHeight, vpWidth, vpOffsetX, vpOffsetY;
	if (realAspect > movieAspect) {
		vpHeight = realWinHeight;
		vpWidth = (int) (realWinHeight * movieAspect);
		vpOffsetY = 0;
		vpOffsetX = (realWinWidth-vpWidth)/2;
	} else {
		vpWidth = realWinWidth;
		vpHeight = (int)((float) realWinWidth / movieAspect);
		vpOffsetY = (realWinHeight-vpHeight)/2;
		vpOffsetX = 0;
	}
	
	glViewport (vpOffsetX, vpOffsetY, vpWidth, vpHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 400, 0, 1.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
}

static uint64_t xiph_lace_value(unsigned char ** np)
{
	uint64_t lace;
	uint64_t value;
	unsigned char * p = *np;
	
	lace = *p++;
	value = lace;
	while (lace == 255) {
		lace = *p++;
		value += lace;
	}
	
	*np = p;
	
	return value;
}


vorbis_dsp_state vorbisDspState;
long long audioChannels;
//ALubyte audioBuffer[1024*1024];

// send audio to audio device... 
ALuint feedAudio (void *userdata, ALubyte *data, ALuint length) {
	return 0;
}

int playMovie (int fileNumber)
{
	if (movieIsPlaying) return 0;
	
    vpx_codec_ctx_t  codec;
	float pausefade = 1.0;

    using namespace mkvparser;
	
    MkvReader reader;
	
    if (reader.Open(fileNumber))
    {
        warning(ERROR_MOVIE_ODDNESS);
        return 0;
    }
	
    long long pos = 0;
	
    EBMLHeader ebmlHeader;
	
    ebmlHeader.Parse(&reader, pos);
		
    mkvparser::Segment* pSegment;
	
    long long ret = mkvparser::Segment::CreateInstance(&reader, pos, pSegment);
    if (ret)
    {
        fatal ("Movie error: Segment::CreateInstance() failed.\n");
    }
	
    ret  = pSegment->Load();
    if (ret < 0)
    {
        fatal ("Movie error: Segment::Load() failed.\n");
    }
	
    //const SegmentInfo* const pSegmentInfo = pSegment->GetInfo();
	
    //const long long timeCodeScale = pSegmentInfo->GetTimeCodeScale();
    //const long long duration_ns = pSegmentInfo->GetDuration();
	
    //const char* const pTitle = pSegmentInfo->GetTitleAsUTF8();	
    //const char* const pMuxingApp = pSegmentInfo->GetMuxingAppAsUTF8();
    //const char* const pWritingApp = pSegmentInfo->GetWritingAppAsUTF8();
		
    //const double duration_sec = double(duration_ns) / 1000000000;
	
    const mkvparser::Tracks* pTracks = pSegment->GetTracks();
	
    unsigned long i = 0;
    const unsigned long j = pTracks->GetTracksCount();
	
    enum { VIDEO_TRACK = 1, AUDIO_TRACK = 2 };
	int videoTrack = -1;
	int audioTrack = -1;
	long long audioBitDepth;
	double audioSampleRate;
	int audioIndex;
	bool soundPlaying = false;
	ogg_packet oggPacket;
	vorbis_info vorbisInfo;
	vorbis_comment vorbisComment;
	vorbis_block vorbisBlock;
		
    while (i != j)
    {
        const Track* const pTrack = pTracks->GetTrackByIndex(i++);
		
        if (pTrack == NULL)
            continue;
		
        const long long trackType = pTrack->GetType();
        //const long long trackNumber = pTrack->GetNumber();
        //const unsigned long long trackUid = pTrack->GetUid();
        //const char* pTrackName = pTrack->GetNameAsUTF8();
								
        if (trackType == VIDEO_TRACK && videoTrack < 0)
        {
			videoTrack = pTrack->GetNumber();
            const VideoTrack* const pVideoTrack =
			static_cast<const VideoTrack*>(pTrack);
			
            const long long width =  pVideoTrack->GetWidth();
            const long long height = pVideoTrack->GetHeight();
			
            //const double rate = pVideoTrack->GetFrameRate();
			
			movieAspect = (float)width/height;
        }
		
        if (trackType == AUDIO_TRACK)
        {
			audioTrack = pTrack->GetNumber();
            const AudioTrack* const pAudioTrack =
			static_cast<const AudioTrack*>(pTrack);
			
            audioChannels =  pAudioTrack->GetChannels();
            audioBitDepth = pAudioTrack->GetBitDepth();
            audioSampleRate = pAudioTrack->GetSamplingRate();
									
			size_t audioHeaderSize;
			const unsigned char* audioHeader = pAudioTrack->GetCodecPrivate(audioHeaderSize);
			
			if (audioHeaderSize < 1) {
				warning("Strange audio track in movie.");
				audioTrack = -1;
				continue;
			}
			
			unsigned char *p = (unsigned char *)audioHeader;

			unsigned int count = *p++ + 1;
			if (count != 3) {
				warning("Strange audio track in movie.");
				audioTrack = -1;
				continue;
			}

			uint64_t sizes[3], total;

			int i = 0;
			total = 0;
			while (--count) {
				sizes[i] = xiph_lace_value(&p);
				total += sizes[i];
				i += 1;
			}
			sizes[i] = audioHeaderSize - total- (p - audioHeader);
			
			// initialize vorbis 
			vorbis_info_init(&vorbisInfo); 
			vorbis_comment_init(&vorbisComment); 
			memset(&vorbisDspState,0,sizeof(vorbisDspState)); 
			memset(&vorbisBlock,0,sizeof(vorbisBlock)); 

			oggPacket.e_o_s = false; 
			oggPacket.granulepos = 0; 
			oggPacket.packetno = 0; 
			int r; 
			for(int i=0;i<3;i++) 
			{ 
				oggPacket.packet = p; 
				oggPacket.bytes = sizes[i]; 
				oggPacket.b_o_s = oggPacket.packetno==0; 
				r = vorbis_synthesis_headerin(&vorbisInfo, &vorbisComment, &oggPacket); 
				if( r ) 
					fprintf(stderr,"vorbis_synthesis_headerin failed, error: %d", r); 
				oggPacket.packetno++; 
				p += sizes[i];
			} 
			
			r = vorbis_synthesis_init(&vorbisDspState, &vorbisInfo); 
			if( r ) 
				fprintf(stderr,"vorbis_synthesis_init failed, error: %d", r); 
			r = vorbis_block_init(&vorbisDspState, &vorbisBlock); 
			if( r ) 
				fprintf(stderr,"vorbis_block_init failed, error: %d", r); 
			
			ALenum audioFormat = alureGetSampleFormat(audioChannels, 16, 0);
			audioIndex = initMovieSound(fileNumber, audioFormat, audioChannels, (ALuint) audioSampleRate, feedAudio);
        }
    }
	
	if (videoTrack < 0)
		fatal ("Movie error: No video in movie file.");
	
    const unsigned long clusterCount = pSegment->GetCount();
		
    if (clusterCount == 0)
    {
        fatal ("Movie error: Segment has no clusters.\n");
    }
	
	/* Initialize video codec */                                                    //
    if(vpx_codec_dec_init(&codec, interface, NULL, 0))                    //
        die_codec(&codec, "Failed to initialize decoder for movie.");         //	

    unsigned char *frame = new unsigned char[256*1024];
	GLubyte * ytex = NULL;
	GLubyte * utex = NULL;
	GLubyte * vtex = NULL;
	GLuint yTextureName = 0;
	GLuint uTextureName = 0;
	GLuint vTextureName = 0;
	
    const mkvparser::Cluster* pCluster = pSegment->GetFirst();
	
					
	
	Init_Special_Timer(24);
	
	setMovieViewport();
	
	movieIsPlaying = 1;
	
	glDepthMask (GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	
	//const long long timeCode = pCluster->GetTimeCode();
	long long time_ns = pCluster->GetTime();
	
	const BlockEntry* pBlockEntry = pCluster->GetFirst();

	if  ((pBlockEntry == NULL) || pBlockEntry->EOS())
	{
		pCluster = pSegment->GetNext(pCluster);
		if ((pCluster == NULL) || pCluster->EOS()) {
			fatal("Error: No movie found in the movie file.");
		}
		pBlockEntry = pCluster->GetFirst();
	}
	const Block* pBlock  = pBlockEntry->GetBlock();
	long long trackNum = pBlock->GetTrackNumber();
	unsigned long tn = static_cast<unsigned long>(trackNum);
	const Track* pTrack = pTracks->GetTrackByNumber(tn);
	long long trackType = pTrack->GetType();
	int frameCount = pBlock->GetFrameCount();
	time_ns = pBlock->GetTime(pCluster);
	
	int frameCounter = 0;
	
	while ( movieIsPlaying ) {
		
		checkInput();
		if (weAreDoneSoQuit)
			break;
		handleInput ();
		
		if (movieIsPlaying == 1) {
			// Play the movie!
			
			if  ((pCluster != NULL) && !pCluster->EOS())
			{
				
				if (frameCounter >= frameCount) {
					
					pBlockEntry = pCluster->GetNext(pBlockEntry);
					if  ((pBlockEntry == NULL) || pBlockEntry->EOS())
					{
						pCluster = pSegment->GetNext(pCluster);
						if ((pCluster == NULL) || pCluster->EOS()) {
							goto movieHasEnded;
						}
						pBlockEntry = pCluster->GetFirst();
					}
					pBlock  = pBlockEntry->GetBlock();
					trackNum = pBlock->GetTrackNumber();
					tn = static_cast<unsigned long>(trackNum);
					pTrack = pTracks->GetTrackByNumber(tn);
					trackType = pTrack->GetType();
					frameCount = pBlock->GetFrameCount();
					time_ns = pBlock->GetTime(pCluster);
					
					frameCounter = 0;
				}
				
				const Block::Frame& theFrame = pBlock->GetFrame(frameCounter);
				const long size = theFrame.len;
				//                const long long offset = theFrame.pos;
				
				if( size > sizeof(frame) ) { 
					if( frame ) delete [] frame; 
					frame = new unsigned char[size]; 
					if (! checkNew(frame)) return 0;
				} 
				/*

				fprintf (stderr, "Block :%s,%s,%15lld\n",
				 (trackType == VIDEO_TRACK) ? "V" : "A",
				 pBlock->IsKey() ? "I" : "P",
				 time_ns);
*/
				if (trackNum == videoTrack) {
					
					theFrame.Read(&reader, frame);
						
					/* Decode the frame */                                                
					if(vpx_codec_decode(&codec, frame, size, NULL, 0))                //
						die_codec(&codec, "Failed to decode frame");

					// Let's decode an image frame!
					vpx_codec_iter_t  iter = NULL;
					vpx_image_t      *img;
					/* Get frame data */
					while((img = vpx_codec_get_frame(&codec, &iter))) {						
						if (img->fmt != VPX_IMG_FMT_I420)
							fatal("Movie error. The movie is not in I420 colour format, which is the only one I can hanlde at the moment.");
						
						unsigned int y;						
						
						if (! ytex) {
							ytex = new GLubyte [img->d_w * img->d_h];
							utex = new GLubyte [(img->d_w >> 1) * (img->d_h >> 1)];
							vtex = new GLubyte [(img->d_w >> 1) * (img->d_h >> 1)];
							if (!ytex || !utex || !vtex)
								fatal (ERROR_OUT_OF_MEMORY);
							
							glEnable (GL_TEXTURE_2D);
							if (! yTextureName) glGenTextures (1, &yTextureName);
							if (! uTextureName) glGenTextures (1, &uTextureName);
							if (! vTextureName) glGenTextures (1, &vTextureName);
							glBindTexture (GL_TEXTURE_2D, yTextureName);
							glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, img->d_w, img->d_h, 0,
										 GL_ALPHA, GL_UNSIGNED_BYTE, ytex);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
							glBindTexture (GL_TEXTURE_2D, uTextureName);
							glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, img->d_w>>1, img->d_h>>1, 0,
										 GL_ALPHA, GL_UNSIGNED_BYTE, utex);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
							glBindTexture (GL_TEXTURE_2D, vTextureName);
							glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, img->d_w>>1, img->d_h>>1, 0,
										 GL_ALPHA, GL_UNSIGNED_BYTE, vtex);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						}
						
						unsigned char *buf =img->planes[0];  
						for(y=0; y<img->d_h; y++) {
							memcpy(ytex+y*img->d_w, buf, img->d_w);  
							buf += img->stride[0];                                
						}                                                             
						buf =img->planes[1];                       
						for(y=0; y<img->d_h >> 1; y++) {                    
							memcpy(utex+y*(img->d_w>>1), buf, img->d_w>>1);  
							buf += img->stride[1];                                
						}                                                             
						buf =img->planes[2];                       
						for(y=0; y<img->d_h >> 1; y++) {                    
							memcpy(vtex+y*(img->d_w>>1), buf, img->d_w>>1);  
							buf += img->stride[2];                                
						}
						glBindTexture (GL_TEXTURE_2D, yTextureName);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->d_w, img->d_h, 
										GL_ALPHA, GL_UNSIGNED_BYTE, ytex);
						glBindTexture (GL_TEXTURE_2D, uTextureName);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->d_w>>1, img->d_h>>1, 
										GL_ALPHA, GL_UNSIGNED_BYTE, utex);
						glBindTexture (GL_TEXTURE_2D, vTextureName);
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img->d_w>>1, img->d_h>>1, 
										GL_ALPHA, GL_UNSIGNED_BYTE, vtex);
						
							
					}
				} else if (trackNum == audioTrack) {
					// Use this Audio Track 
					if( size > 0 ) { 
						theFrame.Read(&reader, frame);
						oggPacket.packet = frame; 
						oggPacket.bytes = size;
						oggPacket.b_o_s = false; 
						oggPacket.packetno++; 
						oggPacket.granulepos = -1; 
						if( ! vorbis_synthesis(&vorbisBlock, &oggPacket) ) { 
							if (vorbis_synthesis_blockin(&vorbisDspState, &vorbisBlock))
								fprintf (stderr, "Vorbis Synthesis block in error.\n");
								
						} else {
							fprintf (stderr, "Vorbis Synthesis error.\n");
						}

						float **pcm;
						
						int numSamples = vorbis_synthesis_pcmout(&vorbisDspState, &pcm);

						if (numSamples > 0) {
							int word = 2;
							int sgned = 1;
							int i, j;
							long bytespersample=audioChannels*word;
							vorbis_fpu_control fpu;
							
							char * buffer = new char[bytespersample*numSamples];
							
							/* a tight loop to pack each size */
							{
								int val;
								if(word==1){
									int off=(sgned?0:128);
									vorbis_fpu_setround(&fpu);
									for(j=0;j<numSamples;j++)
										for(i=0;i<audioChannels;i++){
											val=vorbis_ftoi(pcm[i][j]*128.f);
											if(val>127)val=127;
											else if(val<-128)val=-128;
											*buffer++=val+off;
										}
									vorbis_fpu_restore(fpu);
								}else{
									int off=(sgned?0:32768);
									
									if(sgned){
										
										vorbis_fpu_setround(&fpu);
										for(i=0;i<audioChannels;i++) { /* It's faster in this order */
											float *src=pcm[i];
											short *dest=((short *)buffer)+i;
											for(j=0;j<numSamples;j++) {
												val=vorbis_ftoi(src[j]*32768.f);
												if(val>32767)val=32767;
												else if(val<-32768)val=-32768;
												*dest=val;
												dest+=audioChannels;
											}
										}
										vorbis_fpu_restore(fpu);
										
									}else{
										
										vorbis_fpu_setround(&fpu);
										for(i=0;i<audioChannels;i++) {
											float *src=pcm[i];
											short *dest=((short *)buffer)+i;
											for(j=0;j<numSamples;j++) {
												val=vorbis_ftoi(src[j]*32768.f);
												if(val>32767)val=32767;
												else if(val<-32768)val=-32768;
												*dest=val+off;
												dest+=audioChannels;
											}
										}
										vorbis_fpu_restore(fpu);
										
									}
									
								}
							}
							
							vorbis_synthesis_read(&vorbisDspState,numSamples);
							// TODO: Use the buffer here
							delete [] buffer;

							if (! soundPlaying && size > 1) {
								fprintf (stderr, "start sound playing\n");
								playStream (audioIndex, false, false); 
								soundPlaying = true;
							}
						}							
					}
					 
						
				}
				++frameCounter;

				
			} else {
movieHasEnded:	movieIsPlaying = 0;
			}
			
							
		} 

		// Don't update the screen on audio frames
		if (trackNum == videoTrack || movieIsPlaying == 2) {

			// Clear The Screen
			glClear(GL_COLOR_BUFFER_BIT);	
			
			// Display the current frame here
			if (shader.yuv) {
				glUseProgram(shader.yuv);
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glBindTexture (GL_TEXTURE_2D, uTextureName);
				glActiveTexture(GL_TEXTURE2);
				glEnable(GL_TEXTURE_2D);
				glBindTexture (GL_TEXTURE_2D, vTextureName);
				glActiveTexture(GL_TEXTURE0);
			}
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, yTextureName);
			glEnable(GL_BLEND);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glColor4f(1.0, 1.0, 1.0, 1.0);
			
			glBegin(GL_QUADS);
			
			glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.1);
			glTexCoord2f(1.0, 0.0); glVertex3f(640.0, 0.0, 0.1);
			glTexCoord2f(1.0, 1.0); glVertex3f(640.0, 400.0, 0.1);
			glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 400.0, 0.1);
			
			glEnd();
			
			glActiveTexture(GL_TEXTURE1);
			glDisable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE2);
			glDisable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			
			glUseProgram(0);
		
			if (movieIsPlaying == 2) {
				pausefade -= 1.0 / 24;
				if (pausefade<-1.0) pausefade = 1.0;
				
				// Paused.
				glDisable (GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				glColor4f(0.0, 0.0, 0.0, fabs(pausefade));
				
				glBegin(GL_QUADS);
				
				
				glVertex3f(7.0, 7.0, 0.1);
				glVertex3f(17.0, 7.0, 0.1);
				glVertex3f(17.0, 29.0, 0.1);
				glVertex3f(7.0, 29.0, 0.1);
				
				glVertex3f(27.0, 7.0, 0.1);
				glVertex3f(37.0, 7.0, 0.1);
				glVertex3f(37.0, 29.0, 0.1);
				glVertex3f(27.0, 29.0, 0.1);

				glColor4f(1.0, 1.0, 1.0, fabs(pausefade));

				glVertex3f(5.0, 5.0, 0.1);
				glVertex3f(15.0, 5.0, 0.1);
				glVertex3f(15.0, 27.0, 0.1);
				glVertex3f(5.0, 27.0, 0.1);

				glVertex3f(25.0, 5.0, 0.1);
				glVertex3f(35.0, 5.0, 0.1);
				glVertex3f(35.0, 27.0, 0.1);
				glVertex3f(25.0, 27.0, 0.1);
				
				glEnd();
		
				glDisable(GL_BLEND);
				
			}
			glFlush();
			SDL_GL_SwapBuffers();
		
			Wait_Frame();
		}
	}
	
	// Cleanup	
	huntKillFreeSound(fileNumber);
    if(vpx_codec_destroy(&codec))                                             //
        die_codec(&codec, "Failed to destroy codec");                         //
	vorbis_dsp_clear(&vorbisDspState);
	vorbis_block_clear(&vorbisBlock);
	vorbis_comment_clear(&vorbisComment);
	vorbis_info_clear(&vorbisInfo);
    delete pSegment;
	delete ytex;
	delete utex;
	delete vtex;
	glDeleteTextures (1, &yTextureName);
	glDeleteTextures (1, &uTextureName);
	glDeleteTextures (1, &vTextureName);
	movieIsPlaying = 0;
	Init_Timer();
	glViewport (viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);
	setPixelCoords (false);
	return 1;
}

int stopMovie ()
{
	movieIsPlaying = 0;
	return 0;
}

int pauseMovie()
{
	if (movieIsPlaying == 1)
		movieIsPlaying = 2;
	else if (movieIsPlaying == 2)
		movieIsPlaying = 1;
	return movieIsPlaying;
}
