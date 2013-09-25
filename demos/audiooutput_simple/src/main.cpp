#include "dp/cli.h"
#include "dp/common/primitives.h"
#include "dp/common/stringconverter.h"
#include "dp/audio/speakermanager.h"
#include "dp/audio/speakerkey.h"

#include "wav.h"

#include <mutex>
#include <condition_variable>
#include <chrono>
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

    WaveData        waveData;
    dp::AudioFormat audioFormat;
    dp::UInt        sampleRate;
    dp::UInt        channels;
    if( readWav(
        FILE_PATH
        , waveData
        , audioFormat
        , sampleRate
        , channels
    ) == false ) {
        std::printf( "ファイルの解析に失敗\n" );

        return 1;
    }

    //TODO 音声の再生

    return 0;
}
