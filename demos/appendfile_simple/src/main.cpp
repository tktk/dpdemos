#include "dp/cli.h"
#include "dp/common/stringconverter.h"
#include "dp/file/filew.h"

#include "input.h"

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

    auto    fileUnique = dp::unique( dp::newFileA( FILE_PATH ) );
    if( fileUnique.get() == nullptr ) {
        std::printf( "dp::FileWの生成に失敗\n" );

        return 1;
    }
    auto &  file = *fileUnique;

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
            file
            , writeString.c_str()
            , length
        ) == false ) {
            std::printf( "ファイルへの書き込みに失敗\n" );

            return 1;
        }
    }

    return 0;
}
