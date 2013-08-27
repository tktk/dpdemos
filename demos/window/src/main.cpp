#include "dp/cli.h"
#include "dp/window/window.h"
#include "dp/window/windowflags.h"
#include "dp/common/stringconverter.h"

#include <thread>
#include <mutex>
#include <condition_variable>

dp::Bool generateTitle(
    dp::Utf32 &             _title
    , const dp::Utf32 &     _HEADER
    , const dp::String &    _DESCRIPTION
)
{
    _title.assign( _HEADER );

    dp::Utf32   colon;
    if( dp::toUtf32(
        colon
        , " : "
    ) == false ) {
        return false;
    }

    _title.append( colon );

    dp::Utf32   descriptionUtf32;
    if( dp::toUtf32(
        descriptionUtf32
        , _DESCRIPTION
    ) == false ) {
        return false;
    }

    _title.append( descriptionUtf32 );

    return true;
}

void setClose(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _closed
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    _closed = true;

    _cond.notify_all();
}

void waitClose(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , const dp::Bool &          _CLOSED
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    _cond.wait(
        lock
        , [
            &_CLOSED
        ]
        {
            return _CLOSED;
        }
    );
}

struct ThreadProc
{
private:
    const dp::Utf32 &           TITLE;
    const dp::String            DESCRIPTION;
    dp::WindowFlags             flags;
    std::mutex &                mutex;
    std::condition_variable &   cond;

public:
    ThreadProc(
        const dp::Utf32 &           _TITLE
        , const dp::StringChar *    _DESCRIPTION
        , dp::WindowFlags           _flags
        , std::mutex &              _mutex
        , std::condition_variable & _cond
    )
        : TITLE( _TITLE )
        , DESCRIPTION( _DESCRIPTION )
        , flags( _flags )
        , mutex( _mutex )
        , cond( _cond )
    {
    }

    void operator()(
    )
    {
        dp::Utf32   title;

        if( generateTitle(
            title
            , this->TITLE
            , this->DESCRIPTION
        ) == false ) {
            return;
        }

        dp::Bool    closed = false;

        dp::WindowInfoUnique    infoUnique( dp::newWindowInfo() );

        auto &  info = *infoUnique;

        dp::setClosingEventHandler(
            info
            , [
                this
                , &closed
            ]
            (
                dp::Window &
            )
            {
                setClose(
                    this->mutex
                    , this->cond
                    , closed
                );
            }
        );

        //TODO

        dp::WindowUnique    windowUnique(
            dp::newWindow(
                info
                , title
                , 100
                , 100
                , this->flags
            )
        );
        if( windowUnique.get() == nullptr ) {
            return;
        }

        waitClose(
            this->mutex
            , this->cond
            , closed
        );
    }
};

dp::Int dpMain(
    dp::Args &  _args
)
{
    std::mutex              mutex;
    std::condition_variable cond;

    dp::Utf32   title;

    if( _args.size() >= 2 ) {
        title = _args[ 1 ];
    }

    std::thread plain(
        ThreadProc(
            title
            , "PLAIN"
            , dp::WindowFlags::PLAIN
            , mutex
            , cond
        )
    );

    std::thread unresizable(
        ThreadProc(
            title
            , "UNRESIZABLE"
            , dp::WindowFlags::UNRESIZABLE
            , mutex
            , cond
        )
    );

    std::thread alwaysOnTop(
        ThreadProc(
            title
            , "ALWAYS_ON_TOP"
            , dp::WindowFlags::ALWAYS_ON_TOP
            , mutex
            , cond
        )
    );

    plain.join();
    unresizable.join();
    alwaysOnTop.join();

    return 0;
}
