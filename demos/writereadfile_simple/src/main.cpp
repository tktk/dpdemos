#include "dp/cli.h"
#include "dp/common/stringconverter.h"
#include "dp/file/filerw.h"

#include "input.h"

#include <cstdio>

dp::Bool write(
    dp::FileRW &    _file
)
{
    dp::String  writeString;
    while( 1 ) {
        input(
            writeString
        );

        dp::ULong   length = writeString.size();
        if( length <= 0 ) {
            break;
        }

        if( dp::write(
            _file
            , writeString.c_str()
            , length
        ) == false ) {
            std::printf( "ファイルへの書き込みに失敗\n" );

            return false;
        }
    }

    return true;
}

dp::Bool read(
    dp::FileRW &    _file
)
{
    const auto  BUFFER_SIZE = 10;

    std::vector< dp::StringChar >   buffer( BUFFER_SIZE );

    auto    bufferPtr = buffer.data();

    while( 1 ) {
        dp::ULong   bufferSize = buffer.size();
        if( dp::read(
            _file
            , bufferPtr
            , bufferSize
        ) == false ) {
            std::printf( "ファイルからの読み込みに失敗\n" );

            return false;
        }

        if( bufferSize <= 0 ) {
            break;
        }

        std::printf(
            "%.*s"
            , bufferSize
            , bufferPtr
        );
    }

    return true;
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

    auto    fileUnique = dp::unique( dp::newFileWR( FILE_PATH ) );
    if( fileUnique.get() == nullptr ) {
        std::printf( "dp::FileRWの生成に失敗\n" );

        return 1;
    }
    auto &  file = *fileUnique;

    if( write(
        file
    ) == false ) {
        return 1;
    }

    std::printf( "\n" );

    if( dp::setPosition(
        file
        , 0
    ) == false ) {
        std::printf( "ファイルポインタの移動に失敗\n" );

        return 1;
    }

    if( read(
        file
    ) == false ) {
        return 1;
    }

    return 0;
}
