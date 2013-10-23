#include "dp/cli.h"
#include "dp/common/stringconverter.h"
#include "dp/file/filew.h"

#include <cstdio>

dp::Bool toLong(
    dp::Long &          _long
    , const dp::Utf32 & _UTF32
)
{
    dp::String  str;
    if( dp::toString(
        str
        , _UTF32
    ) == false ) {
        return false;
    }

    dp::StringChar *    endPtr = nullptr;
    _long = std::strtoll(
        str.c_str()
        , &endPtr
        , 10
    );
    if( *endPtr != '\0' ) {
        return false;
    }

    return true;
}

dp::Int dpMain(
    dp::Args &  _args
)
{
    if( _args.size() < 3 ) {
        dp::String  command;
        dp::toString(
            command
            , _args[ 0 ]
        );

        std::printf( "使い方: %s ファイルパス ファイルサイズ\n", command.c_str() );

        return 1;
    }

    const auto &    FILE_PATH = _args[ 1 ];
    const auto &    FILE_SIZE_STR = _args[ 2 ];

    dp::Long    fileSize;
    if( toLong(
        fileSize
        , FILE_SIZE_STR
    ) == false ) {
        std::printf( "ファイルサイズの数値変換に失敗\n" );

        return 1;
    }

    auto    fileUnique = dp::unique( dp::newFileA( FILE_PATH ) );
    if( fileUnique.get() == nullptr ) {
        std::printf( "dp::FileWの生成に失敗\n" );

        return 1;
    }
    auto &  file = *fileUnique;

    dp::truncate(
        file
        , fileSize
    );

    return 0;
}
