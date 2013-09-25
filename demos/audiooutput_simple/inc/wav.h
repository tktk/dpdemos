#ifndef AUDIOOUTPUT_SIMPLE_WAV_H
#define AUDIOOUTPUT_SIMPLE_WAV_H

#include "dp/audio/audioformat.h"
#include "dp/common/primitives.h"

#include <vector>

typedef std::vector< dp::Byte > WaveData;

dp::Bool readWav(
    const dp::Utf32 &
    , WaveData &
    , dp::AudioFormat &
    , dp::UInt &
    , dp::UInt &
);

#endif  // AUDIOOUTPUT_SIMPLE_WAV_H
