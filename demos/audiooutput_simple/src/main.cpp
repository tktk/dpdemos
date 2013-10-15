#include "dp/cli.h"
#include "dp/common/primitives.h"
#include "dp/common/stringconverter.h"
#include "dp/audio/speakermanager.h"
#include "dp/audio/speakerkey.h"
#include "dp/audio/audioformat.h"
#include "dp/audio/audioplayer.h"

#include "wav.h"

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstring>
#include <cstdio>

void waitForFindSpeakerKey(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , dp::SpeakerKeyUnique &    _keyUnique
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    _cond.wait_for(
        lock
        , std::chrono::seconds( 5 )
        , [
            &_keyUnique
        ]
        {
            return _keyUnique.get() != nullptr;
        }
    );
}

void foundSpeakerKey(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , dp::SpeakerKeyUnique &    _keyUnique
    , dp::SpeakerKeyUnique &    _foundKeyUnique
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    _keyUnique = std::move( _foundKeyUnique );

    _cond.notify_one();
}

dp::SpeakerKey * getSpeakerKey(
)
{
    std::mutex              mutex;
    std::condition_variable cond;
    dp::SpeakerKeyUnique    keyUnique;

    dp::SpeakerManagerInfoUnique    infoUnique( dp::newSpeakerManagerInfo() );
    if( infoUnique.get() == nullptr ) {
        std::printf( "dp::SpeakerManagerInfoの生成に失敗\n" );

        return nullptr;
    }
    auto &  info = *infoUnique;

    dp::setConnectEventHandler(
        info
        , [
            &mutex
            , &cond
            , &keyUnique
        ]
        (
            dp::SpeakerManager &        _manager
            , dp::SpeakerKeyUnique &&   _keyUnique
            , dp::Bool                  _connected
        )
        {
            if( _connected == false ) {
                return;
            }

            foundSpeakerKey(
                mutex
                , cond
                , keyUnique
                , _keyUnique
            );

            auto &  info = dp::getInfoMutable( _manager );
            dp::setConnectEventHandler(
                info
                , nullptr
            );
        }
    );

    dp::SpeakerManagerUnique    managerUnique(
        dp::newSpeakerManager(
            info
        )
    );
    if( managerUnique.get() == nullptr ) {
        std::printf( "dp::SpeakerManagerの生成に失敗\n" );

        return nullptr;
    }

    std::printf( "スピーカーを検索中…\n" );

    waitForFindSpeakerKey(
        mutex
        , cond
        , keyUnique
    );

    return keyUnique.release();
}

void waitEnd(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , const dp::Bool &          _ENDED
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    _cond.wait(
        lock
        , [
            &_ENDED
        ]
        {
            return _ENDED;
        }
    );
}

void playAudio(
    const dp::SpeakerKey &  _KEY
    , dp::AudioFormat       _audioFormat
    , dp::UInt              _sampleRate
    , dp::UInt              _channels
    , const WaveData &      _WAVE_DATA
)
{
    std::mutex              mutex;
    std::condition_variable cond;
    dp::Bool                ended = false;

    dp::AudioPlayerInfoUnique   infoUnique( dp::newAudioPlayerInfo() );
    if( infoUnique.get() == nullptr ) {
        std::printf( "dp::AudioPlayerInfoの生成に失敗\n" );

        return;
    }
    auto &  info = *infoUnique;

    auto        waveDataPtr = _WAVE_DATA.data();
    const auto  END_OF_WAVE_DATA = waveDataPtr + _WAVE_DATA.size();

    dp::setStartEventHandler(
        info
        , [
        ]
        (
            dp::AudioPlayer &   _audioPlayer
        )
        {
            dp::pause(
                _audioPlayer
                , false
            );
        }
    );
    dp::setEndEventHandler(
        info
        , [
            &mutex
            , &cond
            , &ended
        ]
        (
            dp::AudioPlayer &
        )
        {
            std::unique_lock< std::mutex >  lock( mutex );

            ended = true;

            cond.notify_one();
        }
    );
    dp::setPlayEventHandler(
        info
        , [
            &waveDataPtr
            , END_OF_WAVE_DATA
        ]
        (
            dp::AudioPlayer &
            , void *            _buffer
            , dp::ULong         _bufferSize
        ) -> dp::ULong
        {
            if( waveDataPtr >= END_OF_WAVE_DATA ) {
                return 0;
            }

            const auto  WAVE_DATA_SIZE = END_OF_WAVE_DATA - waveDataPtr;
            if( _bufferSize > WAVE_DATA_SIZE ) {
                _bufferSize = WAVE_DATA_SIZE;
            }

            std::memcpy(
                _buffer
                , waveDataPtr
                , _bufferSize
            );

            waveDataPtr += _bufferSize;

            return _bufferSize;
        }
    );

    dp::AudioPlayerUnique   audioPlayerUnique(
        dp::newAudioPlayer(
            _KEY
            , info
            , _audioFormat
            , _sampleRate
            , _channels
        )
    );
    if( audioPlayerUnique.get() == nullptr ) {
        std::printf( "dp::AudioPlayerの生成に失敗\n" );

        return;
    }

    waitEnd(
        mutex
        , cond
        , ended
    );
}

dp::Int dpMain(
    dp::Args &  _args
)
{
    if( _args.size() < 2 ) {
        dp::String  command;
        dp::toString(
            command
            , _args[ 0 ]
        );

        std::printf( "使い方: %s ファイルパス\n", command.c_str() );

        return 1;
    }

    const auto &    FILE_PATH = _args[ 1 ];

    dp::SpeakerKeyUnique    keyUnique( getSpeakerKey() );
    if( keyUnique.get() == nullptr ) {
        std::printf( "スピーカーの検索に失敗\n" );

        return 1;
    }
    const auto &    KEY = *keyUnique;

    dp::AudioFormat audioFormat;
    dp::UInt        sampleRate;
    dp::UInt        channels;
    WaveData        waveData;
    if( readWav(
        FILE_PATH
        , audioFormat
        , sampleRate
        , channels
        , waveData
    ) == false ) {
        std::printf( "ファイルの解析に失敗\n" );

        return 1;
    }

    playAudio(
        KEY
        , audioFormat
        , sampleRate
        , channels
        , waveData
    );
    waveData = std::move( WaveData() );

    return 0;
}
