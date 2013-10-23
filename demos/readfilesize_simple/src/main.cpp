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

    if( dp::setPositionFromEnd(
        file
        , 0
    ) == false ) {
        std::printf( "ファイルポインタの移動に失敗\n" );

        return 1;
    }

    dp::Long    position;
    if( dp::getPosition(
        file
        , position
    ) == false ) {
        std::printf( "ファイルポインタの現在位置取得に失敗\n" );

        return 1;
    }

    std::printf( "%lld\n", position );

    return 0;
}
