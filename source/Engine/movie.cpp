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

#include "newfatal.h"
#include "timing.h"
#include "graphics.h"
#include "movie.h"

// in main.c
int checkInput();
extern int weAreDoneSoQuit;

// Sludger.cpp
bool handleInput ();

/*
 movieIsPlaying tracks the state of movie playing
 0 = no movie
 1 = movie is played
 2 = paused
*/
int movieIsPlaying = 0;

float movieAspect = 1.6;

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


int playMovie (int fileNumber)
{
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
	
    fprintf (stderr, "\t\t\t    EBML Header\n");
    fprintf (stderr, "\t\tEBML Version\t\t: %lld\n", ebmlHeader.m_version);
    fprintf (stderr, "\t\tEBML MaxIDLength\t: %lld\n", ebmlHeader.m_maxIdLength);
    fprintf (stderr, "\t\tEBML MaxSizeLength\t: %lld\n", ebmlHeader.m_maxSizeLength);
    fprintf (stderr, "\t\tDoc Type\t\t: %s\n", ebmlHeader.m_docType);
    fprintf (stderr, "\t\tPos\t\t\t: %lld\n", pos);
	
    mkvparser::Segment* pSegment;
	
    long long ret = mkvparser::Segment::CreateInstance(&reader, pos, pSegment);
    if (ret)
    {
        fprintf (stderr, "\n Segment::CreateInstance() failed.\n");
        return -1;
    }
	
    ret  = pSegment->Load();
    if (ret < 0)
    {
        fprintf (stderr, "\n Segment::Load() failed.\n");
        return -1;
    }
	
    const SegmentInfo* const pSegmentInfo = pSegment->GetInfo();
	
    const long long timeCodeScale = pSegmentInfo->GetTimeCodeScale();
    const long long duration_ns = pSegmentInfo->GetDuration();
	
    const char* const pTitle = pSegmentInfo->GetTitleAsUTF8();	
    const char* const pMuxingApp = pSegmentInfo->GetMuxingAppAsUTF8();
    const char* const pWritingApp = pSegmentInfo->GetWritingAppAsUTF8();
	
    fprintf (stderr, "\n");
    fprintf (stderr, "\t\t\t   Segment Info\n");
    fprintf (stderr, "\t\tTimeCodeScale\t\t: %lld \n", timeCodeScale);
    fprintf (stderr, "\t\tDuration\t\t: %lld\n", duration_ns);
	
    const double duration_sec = double(duration_ns) / 1000000000;
    fprintf (stderr, "\t\tDuration(secs)\t\t: %7.3lf\n", duration_sec);
	
	fprintf (stderr, "\t\tTrack Name\t\t: %s\n", pTitle);
	fprintf (stderr, "\t\tMuxing App\t\t: %s\n", pMuxingApp);
	fprintf (stderr, "\t\tWriting App\t\t: %s\n", pWritingApp);
	
    // pos of segment payload
    fprintf (stderr, "\t\tPosition(Segment)\t: %lld\n", pSegment->m_start);
	
    // size of segment payload
    fprintf (stderr, "\t\tSize(Segment)\t\t: %lld\n", pSegment->m_size);
	
    const mkvparser::Tracks* pTracks = pSegment->GetTracks();
	
    unsigned long i = 0;
    const unsigned long j = pTracks->GetTracksCount();
	
    enum { VIDEO_TRACK = 1, AUDIO_TRACK = 2 };
	
    fprintf (stderr, "\n\t\t\t   Track Info\n");
	
    while (i != j)
    {
        const Track* const pTrack = pTracks->GetTrackByIndex(i++);
		
        if (pTrack == NULL)
            continue;
		
        const long long trackType = pTrack->GetType();
        const long long trackNumber = pTrack->GetNumber();
        const unsigned long long trackUid = pTrack->GetUid();
        const char* pTrackName = pTrack->GetNameAsUTF8();
		
        fprintf (stderr, "\t\tTrack Type\t\t: %lld\n", trackType);
        fprintf (stderr, "\t\tTrack Number\t\t: %lld\n", trackNumber);
        fprintf (stderr, "\t\tTrack Uid\t\t: %lld\n", trackUid);		
		fprintf (stderr, "\t\tTrack Name\t\t: %s \n", pTrackName);
				
		fprintf (stderr, "\t\tCodec Id\t\t: %s\n", pTrack->GetCodecId());
		fprintf (stderr, "\t\tCodec Name\t\t: %s\n", pTrack->GetCodecNameAsUTF8());
		
        if (trackType == VIDEO_TRACK)
        {
            const VideoTrack* const pVideoTrack =
			static_cast<const VideoTrack*>(pTrack);
			
            const long long width =  pVideoTrack->GetWidth();
            fprintf (stderr, "\t\tVideo Width\t\t: %lld\n", width);
			
            const long long height = pVideoTrack->GetHeight();
            fprintf (stderr, "\t\tVideo Height\t\t: %lld\n", height);
			
            const double rate = pVideoTrack->GetFrameRate();
            fprintf (stderr, "\t\tVideo Rate\t\t: %f\n",rate);
			
			movieAspect = (float)width/height;
        }
		
        if (trackType == AUDIO_TRACK)
        {
            const AudioTrack* const pAudioTrack =
			static_cast<const AudioTrack*>(pTrack);
			
            const long long channels =  pAudioTrack->GetChannels();
            fprintf (stderr, "\t\tAudio Channels\t\t: %lld\n", channels);
			
            const long long bitDepth = pAudioTrack->GetBitDepth();
            fprintf (stderr, "\t\tAudio BitDepth\t\t: %lld\n", bitDepth);
			
            const double sampleRate = pAudioTrack->GetSamplingRate();
            fprintf (stderr, "\t\tAddio Sample Rate\t: %.3f\n", sampleRate);
        }
    }	
	
    fprintf (stderr, "\n\n\t\t\t   Cluster Info\n");
    const unsigned long clusterCount = pSegment->GetCount();
	
    fprintf (stderr, "\t\tCluster Count\t: %ld\n\n", clusterCount);
	
    if (clusterCount == 0)
    {
        fprintf (stderr, "\t\tSegment has no clusters.\n");
        delete pSegment;
        return -1;
    }
	
    const mkvparser::Cluster* pCluster = pSegment->GetFirst();
	
    while ((pCluster != NULL) && !pCluster->EOS())
    {
        const long long timeCode = pCluster->GetTimeCode();
        fprintf (stderr, "\t\tCluster Time Code\t: %lld\n", timeCode);
		
        const long long time_ns = pCluster->GetTime();
        fprintf (stderr, "\t\tCluster Time (ns)\t: %lld\n", time_ns);
		
        const BlockEntry* pBlockEntry = pCluster->GetFirst();
		
        while ((pBlockEntry != NULL) && !pBlockEntry->EOS())
        {
            const Block* const pBlock  = pBlockEntry->GetBlock();
            const long long trackNum = pBlock->GetTrackNumber();
            const unsigned long tn = static_cast<unsigned long>(trackNum);
            const Track* const pTrack = pTracks->GetTrackByNumber(tn);
            const long long trackType = pTrack->GetType();
            const int frameCount = pBlock->GetFrameCount();
            const long long time_ns = pBlock->GetTime(pCluster);
			
            fprintf (stderr, "\t\t\tBlock\t\t:%s,%s,%15lld\n",
                   (trackType == VIDEO_TRACK) ? "V" : "A",
                   pBlock->IsKey() ? "I" : "P",
                   time_ns);
			
            for (int i = 0; i < frameCount; ++i)
            {
                const Block::Frame& theFrame = pBlock->GetFrame(i);
                const long size = theFrame.len;
                const long long offset = theFrame.pos;
                fprintf (stderr, "\t\t\t %15ld,%15llx\n", size, offset);
            }
			
            pBlockEntry = pCluster->GetNext(pBlockEntry);
        }
		
        pCluster = pSegment->GetNext(pCluster);
    }
	
    delete pSegment;
	
	
	
	Init_Special_Timer(24);
	
	setMovieViewport();
	
	movieIsPlaying = 1;
	
	glDepthMask (GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	while ( movieIsPlaying ) {
		
		checkInput();
		if (weAreDoneSoQuit)
			break;
		handleInput ();
		
		if (movieIsPlaying == 1) {
			// Play the movie!
			
		} 

		// Clear The Screen
		glClear(GL_COLOR_BUFFER_BIT);	
		
		// Display the current frame here
		glDisable (GL_TEXTURE_2D);
		glColor4f(0.0, 0.5, 0.0, 1.0);
		
		glBegin(GL_QUADS);
		
		glVertex3f(0.0, 0.0, 0.1);
		glVertex3f(640.0, 0.0, 0.1);
		glVertex3f(640.0, 400.0, 0.1);
		glVertex3f(0.0, 400.0, 0.1);
		
		glEnd();
		
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
	
	// Cleanup
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
