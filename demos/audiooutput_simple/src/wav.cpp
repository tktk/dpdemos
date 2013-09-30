#include "wav.h"

#include "dp/audio/audioformat.h"
#include "dp/common/stringconverter.h"
#include "dp/common/primitives.h"

#include <memory>
#include <cstdio>
#include <cstring>

namespace {
    struct CloseFile
    {
        void operator()(
            std::FILE * _file
        )
        {
            std::fclose( _file );
        }
    };

    FILE * openFile(
        const dp::Utf32 &   _FILE_PATH
    )
    {
        dp::String  filePathString;
        dp::toString(
            filePathString
            , _FILE_PATH
        );

        return fopen(
            filePathString.c_str()
            , "rb"
        );
    }

    typedef std::unique_ptr<
        std::FILE
        , CloseFile
    > FileUnique;

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
        std::FILE & _file
    )
    {
        RiffHeader  header;
        if( std::fread(
            &header
            , sizeof( header )
            , 1
            , &_file
        ) != 1 ) {
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
        std::FILE & _file
    )
    {
        WavHeader   header;
        if( std::fread(
            &header
            , sizeof( header )
            , 1
            , &_file
        ) != 1 ) {
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
        std::FILE &         _file
        , const dp::Byte *  _TAG
    )
    {
        RiffChunkHeader header;
        while( 1 ) {
            if( std::fread(
                &header
                , sizeof( header )
                , 1
                , &_file
            ) != 1 ) {
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

            if( std::fseek(
                &_file
                , header.size
                , SEEK_CUR
            ) != 0 ) {
                std::printf( "RIFFチャンクヘッダのシークに失敗\n" );

                return 0;
            }
        }

        return header.size;
    }

    dp::Bool readFmtChunk(
        std::FILE &         _file
        , dp::AudioFormat & _audioFormat
        , dp::UInt &        _sampleRate
        , dp::UInt &        _channels
    )
    {
        const auto  SIZE = findChunk(
            _file
            , TAG_FMT
        );
        if( SIZE <= 0 ) {
            return false;
        }

        std::vector< dp::Byte > buffer( SIZE );
        if( std::fread(
            buffer.data()
            , buffer.size()
            , 1
            , &_file
        ) != 1 ) {
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
        std::FILE &     _file
        , WaveData &    _waveData
    )
    {
        const auto  SIZE = findChunk(
            _file
            , TAG_DATA
        );
        if( SIZE <= 0 ) {
            return false;
        }

        _waveData.resize( SIZE );

        if( std::fread(
            _waveData.data()
            , _waveData.size()
            , 1
            , &_file
        ) != 1 ) {
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
    FileUnique  fileUnique(
        openFile(
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

    const auto  CHUNK_HEAD = std::ftell( &file );

    std::fseek(
        &file
        , CHUNK_HEAD
        , SEEK_SET
    );

    if( readFmtChunk(
        file
        , _audioFormat
        , _sampleRate
        , _channels
    ) == false ) {
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
