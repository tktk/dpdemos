#ifndef COMMON_INPUT_H
#define COMMON_INPUT_H

#include "dp/common/primitives.h"

#include <cstdio>
#include <cstring>

const auto  BUFFER_SIZE = 100;
const auto  PROMPT = "> ";

inline dp::Bool input(
    dp::Int &   _result
)
{
    std::printf( PROMPT );

    dp::StringChar  buffer[ BUFFER_SIZE ];

    std::fgets(
        buffer
        , sizeof( buffer )
        , stdin
    );

    dp::StringChar *    endPtr = nullptr;
    _result = strtol(
        buffer
        , &endPtr
        , 10
    );

    // 1文字以上入力され、入力データが全て変換されているなら正常な入力
    const auto  INPUTED = buffer != endPtr && *endPtr == '\n';

    while( buffer[ std::strlen( buffer ) - 1 ] != '\n' ) {
        std::fgets(
            buffer
            , sizeof( buffer )
            , stdin
        );
    }

    return INPUTED;
}

inline void input(
    dp::String &    _result
)
{
    _result.clear();

    std::printf( PROMPT );

    dp::StringChar  buffer[ BUFFER_SIZE ];

    while( 1 ) {
        std::memset(
            buffer
            , 0x00
            , sizeof( buffer )
        );

        std::fgets(
            buffer
            , sizeof( buffer )
            , stdin
        );

        _result.append( buffer );
        if( _result.size() <= 0 ) {
            break;
        }

        const auto  LENGTH = std::strlen( buffer );
        if( LENGTH > 0 && buffer[ LENGTH - 1 ] == '\n' ) {
            break;
        }
    }
}

#endif  // COMMON_INPUT_H
