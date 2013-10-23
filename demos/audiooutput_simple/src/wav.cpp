#include "wav.h"

#include "dp/audio/audioformat.h"
#include "dp/file/filer.h"
#include "dp/common/stringconverter.h"
#include "dp/common/primitives.h"

#include <cstdio>
#include <cstring>

namespace {
    struct RiffHeader
    {
        dp::Byte    magic[ 4 ];
        dp::UInt    fileSize;
    };

    struct WavHeader
    {
        dp::Byte    magic[ 4 ];
    };

    struct RiffChunkHeader
    {
        dp::Byte    tag[ 4 ];
        dp::UInt    size;
    };

    struct FmtChunk
    {
        dp::UShort  formatId;
        dp::UShort  channels;
        dp::UInt    sampleRate;
        dp::UInt    bytesPerSec;
        dp::UShort  blockSize;
        dp::UShort  bitsPerSample;
    };

    const dp::Byte  MAGIC_RIFF[] = { 'R', 'I', 'F', 'F' };
    const dp::Byte  MAGIC_WAVE[] = { 'W', 'A', 'V', 'E' };

    const dp::Byte  TAG_FMT[] = { 'f', 'm', 't', ' ' };
    const dp::Byte  TAG_DATA[] = { 'd', 'a', 't', 'a' };

    const dp::UShort    FORMAT_ID_LINEAR_PCM = 0x1;

    dp::Bool checkRiffHeader(
        dp::FileR & _file
    )
    {
        RiffHeader  header;

        const auto  HEADER_SIZE = sizeof( header );

        dp::ULong   size = HEADER_SIZE;
        if( dp::read(
            _file
            , &header
            , size
        ) == false ) {
            std::printf( "RIFFヘッダ読み込み処理が失敗\n" );

            return false;
        }

        if( size != HEADER_SIZE ) {
            std::printf( "RIFFヘッダの読み込みに失敗\n" );

            return false;
        }

        if( std::memcmp(
            header.magic
            , MAGIC_RIFF
            , sizeof( header.magic )
        ) != 0 ) {
            std::printf( "RIFFヘッダの識別子が不一致\n" );

            return false;
        }

        return true;
    }

    dp::Bool checkWavHeader(
        dp::FileR & _file
    )
    {
        WavHeader   header;

        const auto  HEADER_SIZE = sizeof( header );

        dp::ULong   size = HEADER_SIZE;
        if( dp::read(
            _file
            , &header
            , size
        ) == false ) {
            std::printf( "WAVEヘッダ読み込み処理が失敗\n" );

            return false;
        }

        if( size != HEADER_SIZE ) {
            std::printf( "WAVEヘッダの読み込みに失敗\n" );

            return false;
        }

        if( std::memcmp(
            header.magic
            , MAGIC_WAVE
            , sizeof( header.magic )
        ) != 0 ) {
            std::printf( "WAVEヘッダの識別子が不一致\n" );

            return false;
        }

        return true;
    }

    dp::UInt findChunk(
        dp::FileR &         _file
        , const dp::Byte *  _TAG
    )
    {
        RiffChunkHeader header;

        const auto  HEADER_SIZE = sizeof( header );

        dp::ULong   size = HEADER_SIZE;
        while( 1 ) {
            if( dp::read(
                _file
                , &header
                , size
            ) == false ) {
                std::printf(
                    "RIFFチャンクヘッダ[%.*s]読み込み処理が失敗\n"
                    , static_cast< dp::Int >( sizeof( header.tag ) )
                    , _TAG
                );

                return 0;
            }

            if( size != HEADER_SIZE ) {
                std::printf(
                    "RIFFチャンクヘッダ[%.*s]の読み込みに失敗\n"
                    , static_cast< dp::Int >( sizeof( header.tag ) )
                    , _TAG
                );

                return 0;
            }

            if( std::memcmp(
                header.tag
                , _TAG
                , sizeof( header.tag )
            ) == 0 ) {
                break;
            }

            if( dp::movePosition(
                _file
                , header.size
            ) == false ) {
                std::printf( "次のRIFFチャンクヘッダへの移動に失敗\n" );

                return 0;
            }
        }

        return header.size;
    }

    dp::Bool readFmtChunk(
        dp::FileR &         _file
        , dp::AudioFormat & _audioFormat
        , dp::UInt &        _sampleRate
        , dp::UInt &        _channels
    )
    {
        const auto  CHUNK_SIZE = findChunk(
            _file
            , TAG_FMT
        );
        if( CHUNK_SIZE <= 0 ) {
            return false;
        }

        dp::ULong   size = CHUNK_SIZE;
        std::vector< dp::Byte > buffer( CHUNK_SIZE );
        if( dp::read(
            _file
            , buffer.data()
            , size
        ) == false ) {
            std::printf( "fmtチャンク読み込み処理が失敗\n" );

            return false;
        }

        if( size != CHUNK_SIZE ) {
            std::printf( "fmtチャンクの読み込みに失敗\n" );

            return false;
        }

        const auto &    FMT_CHUNK = *reinterpret_cast< FmtChunk * >( buffer.data() );

        if( FMT_CHUNK.formatId != FORMAT_ID_LINEAR_PCM ) {
            std::printf( "非対応のフォーマットID\n" );

            return false;
        }

        switch( FMT_CHUNK.bitsPerSample ) {
        case 8:
            _audioFormat = dp::AudioFormat::U8;
            break;

        case 16:
            _audioFormat = dp::AudioFormat::S16LE;
            break;

        default:
            std::printf( "非対応のフォーマット\n" );
            return false;
            break;
        }

        _sampleRate = FMT_CHUNK.sampleRate;
        _channels = FMT_CHUNK.channels;

        return true;
    }

    dp::Bool readDataChunk(
        dp::FileR &     _file
        , WaveData &    _waveData
    )
    {
        const auto  CHUNK_SIZE = findChunk(
            _file
            , TAG_DATA
        );
        if( CHUNK_SIZE <= 0 ) {
            return false;
        }

        dp::ULong   size = CHUNK_SIZE;

        _waveData.resize( CHUNK_SIZE );

        if( dp::read(
            _file
            , _waveData.data()
            , size
        ) != 1 ) {
            std::printf( "波形データ読み込み処理が失敗\n" );

            return false;
        }

        if( size != CHUNK_SIZE ) {
            std::printf( "波形データの読み込みに失敗\n" );

            return false;
        }

        return true;
    }
}

dp::Bool readWav(
    const dp::Utf32 &   _FILE_PATH
    , dp::AudioFormat & _audioFormat
    , dp::UInt &        _sampleRate
    , dp::UInt &        _channels
    , WaveData &        _waveData
)
{
    auto    fileUnique = dp::unique(
        dp::newFileR(
            _FILE_PATH
        )
    );
    if( fileUnique.get() == nullptr ) {
        std::printf( "ファイルのオープンに失敗\n" );

        return false;
    }
    auto &  file = *fileUnique;

    if( checkRiffHeader(
        file
    ) == false ) {
        return false;
    }

    if( checkWavHeader(
        file
    ) == false ) {
        return false;
    }

    dp::Long    chunkHead;
    if( dp::getPosition(
        file
        , chunkHead
    ) == false ) {
        std::printf( "ファイルポインタの現在位置取得に失敗\n" );

        return false;
    }

    if( readFmtChunk(
        file
        , _audioFormat
        , _sampleRate
        , _channels
    ) == false ) {
        return false;
    }

    if( dp::setPosition(
        file
        , chunkHead
    ) == false ) {
        std::printf( "ファイルポインタの移動に失敗\n" );

        return false;
    }

    if( readDataChunk(
        file
        , _waveData
    ) == false ) {
        return false;
    }

    return true;
}
