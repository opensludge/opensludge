#include <allegro.h>
#include <alogg.h>

void playSound(sound) {
	// Load the sound into a SAMPLE

	// ...and play it. 
	voice = play_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop);
}
void loopSound(sound) {
	// see playSound
}
void stopSound(sound) {
	// Just match the sound to its sample and...
	stop_sample(const SAMPLE *spl);
}
void setDefaultSoundVolume(volume) {
	// Just set a flag to use in playSound
}


// Internal Allegro things
#define VIRTUAL_VOICES 256
extern VOICE _voice[VIRTUAL_VOICES];     /* list of active samples */

void setSoundVolume(sound, volume) {
	// adjust_sample only changes the first one it encounters,
	// so we have to change sample volume on the voices instead.
	// I'm lazy, so instead of keeping track on the voices,
	// I use an internal Allegro internal struct
	int c;

  for (c=0; c<VIRTUAL_VOICES; c++)
    if (_voice[c].sample == spl)
			voice_set_volume(c, volume);
}

void setSoundLoopPoints(sound, unsigned long start, unsigned long end) {
	SAMPLE sample;
	sample->loop_start = start;
	sample->loop_end = end;
}


void startMusic(music, channel, pos) {
	// Is it midi?
	//... insert MIDI routine here - I haven't bothered with this,
	// but MIDI will have to be handled separately in all music functions.


	// If it's not midi:
	// First load it in a DUH

	// And play it
	AL_DUH_PLAYER *al_start_duh(DUH *duh, int n_channels, long pos,
                            float volume, long bufsize, int freq);

}
void stopMusic(channel) {
	al_stop_duh(AL_DUH_PLAYER *dp);
	unload_duh(DUH *duh);
}
void setDefaultMusicVolume(volume) {
	// Just set a flag for startMusic
}
void setMusicVolume(volume, channel) {
	al_duh_set_volume(AL_DUH_PLAYER *dp, float volume);
}


// Yes, it's main and not WinMain - also note the END_OF_MAIN() macro
// Allegro needs this beacuse it's multi-platform.
// (On Windows, END_OF_MAIN converts the main into a WinMain.)
int main(int	argc,	char *argv[])
{
	// Initialise Allegro for sound
  allegro_init();
  if (install_sound(DIGI_AUTODETECT,MIDI_NONE,NULL)<0) {
		// error
	}
	install_timer(); // The timer is needed to play MIDIs, maybe for something else too?
  alogg_init();		// Init OGG



	return 0;
}
END_OF_MAIN() 
