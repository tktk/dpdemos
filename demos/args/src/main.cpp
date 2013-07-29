#include "dp/cli.h"
#include "dp/common/stringconverter.h"

#include <cstdio>

dp::Int dpMain(
    dp::Args &  _args
)
{
    for( const auto & ARG : _args ) {
        dp::String  arg;

        if( dp::toString(
            arg
            , ARG
        ) ) {
            std::printf( "%s\n", arg.c_str() );
        } else {
            std::printf( "dp::Stringへの変換に失敗\n" );
        }
    }

    return 0;
}
