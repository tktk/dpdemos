#include "dp/cli.h"
#include "dp/window/window.h"

#include <mutex>
#include <condition_variable>

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
    std::mutex              mutex;
    std::condition_variable cond;
    dp::Bool                ended = false;

    dp::Utf32   title;

    if( _args.size() >= 2 ) {
        title = _args[ 1 ];
    }

    dp::WindowInfoUnique    infoUnique( dp::newWindowInfo() );

    auto &  info = *infoUnique;

    dp::setClosingEventHandler(
        info
        , [
            &mutex
            , &cond
            , &ended
        ]
        (
            dp::Window &
        )
        {
            std::unique_lock< std::mutex >  lock( mutex );

            ended = true;

            cond.notify_all();
        }
    );

    //TODO

    dp::WindowUnique    windowUnique(
        dp::newWindow(
            info
            , title
            , 100
            , 100
        )
    );

    waitEnd(
        mutex
        , cond
        , ended
    );

    return 0;
}
