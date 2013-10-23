#include "dp/cli.h"
#include "dp/common/stringconverter.h"
#include "dp/file/filer.h"

#include <cstdio>

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

    auto    fileUnique = dp::unique( dp::newFileR( FILE_PATH ) );
    if( fileUnique.get() == nullptr ) {
        std::printf( "dp::FileRの生成に失敗\n" );

        return 1;
    }
    auto &  file = *fileUnique;

    const auto  BUFFER_SIZE = 10;

    std::vector< dp::StringChar >   buffer( BUFFER_SIZE );

    auto    bufferPtr = buffer.data();

    while( 1 ) {
        dp::ULong   bufferSize = buffer.size();
        if( dp::read(
            file
            , bufferPtr
            , bufferSize
        ) == false ) {
            std::printf( "ファイルからの読み込みに失敗\n" );

            return 1;
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

    return 0;
}
