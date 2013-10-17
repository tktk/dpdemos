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

    dp::setMouseButtonEventHandler(
        info
        , [](
            dp::Window &
            , dp::ULong     _index
            , dp::Bool      _pressed
        )
        {
            printf(
                "index : %llu, state : %s\n"
                , _index
                , _pressed
                    ? "pressed"
                    : "released"
            );
        }
    );

    dp::setMouseMotionEventHandler(
        info
        , [](
            dp::Window &
            , dp::Int       _x
            , dp::Int       _y
        )
        {
            printf(
                "position : %dx%d\n"
                , _x
                , _y
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
