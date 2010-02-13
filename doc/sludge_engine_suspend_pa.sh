#!/bin/bash
# PulseAudio suspending script for OpenSLUDGE by Tobias Hansen

if [ "$(pidof pulseaudio)" -a "$(which pasuspender)" ]
  then
    echo "Suspending PulseAudio..."
    pasuspender -- sludge_engine "$@"
  else
    sludge_engine "$@"
fi
