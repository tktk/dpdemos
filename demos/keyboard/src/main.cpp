#include "dp/cli.h"
#include "dp/window/window.h"
#include "dp/common/stringconverter.h"

#include <mutex>
#include <condition_variable>
#include <cstdio>

const auto  WIDTH = 100;
const auto  HEIGHT = 100;

dp::Window * newWindow(
    const dp::Utf32 &           _TITLE
    , std::mutex &              _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _ended
)
{
    auto    infoUnique = dp::unique( dp::newWindowInfo() );
    if( infoUnique.get() == nullptr ) {
        std::printf( "dp::WindowInfoの生成に失敗\n" );

        return nullptr;
    }
    auto &  info = *infoUnique;

    dp::setCloseEventHandler(
        info
        , [
            &_mutex
            , &_cond
            , &_ended
        ]
        (
            dp::Window &
        )
        {
            std::unique_lock< std::mutex >  lock( _mutex );

            _ended = true;

            _cond.notify_one();
        }
    );

    dp::setKeyEventHandler(
        info
        , [](
            dp::Window &
            , dp::Key               _key
            , const dp::Utf32Char * _char
            , dp::Bool              _pressed
        )
        {
            dp::String  str;
            if( _char != nullptr ) {
                dp::Utf32   utf32;
                utf32.push_back( *_char );
                dp::toString(
                    str
                    , utf32
                );
            }

            printf(
                "state : %s, key : 0x%x, char : '%s'\n"
                , _pressed
                    ? "pressed"
                    : "released"
                , _key
                , str.c_str()
            );
        }
    );

    return dp::newWindow(
        info
        , _TITLE
        , WIDTH
        , HEIGHT
    );
}

void waitEnd(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , const dp::Bool &          _ENDED
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    _cond.wait(
        lock
        , [
            &_ENDED
        ]
        {
            return _ENDED;
        }
    );
}

dp::Int dpMain(
    dp::Args &  _args
)
{
    dp::Utf32   title;

    if( _args.size() >= 2 ) {
        title = _args[ 1 ];
    }

    std::mutex              mutex;
    std::condition_variable cond;
    dp::Bool                ended = false;

    auto    windowUnique = dp::unique(
        newWindow(
            title
            , mutex
            , cond
            , ended
        )
    );
    if( windowUnique.get() == nullptr ) {
        std::printf( "ウィンドウの生成に失敗\n" );

        return 1;
    }

    waitEnd(
        mutex
        , cond
        , ended
    );

    return 0;
}
