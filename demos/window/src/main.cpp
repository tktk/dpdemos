#include "dp/cli.h"
#include "dp/window/window.h"
#include "dp/window/windowflags.h"
#include "dp/common/stringconverter.h"
#include "dp/common/thread.h"

#include <thread>
#include <mutex>
#include <condition_variable>

const auto  WIDTH = 100;
const auto  HEIGHT = 100;

const auto  X = 1000;
const auto  Y = 1000;

typedef std::function<
    dp::Window * (
        std::mutex &
        , std::condition_variable &
        , dp::Bool &
    )
> NewWindow;

typedef std::function<
    dp::Window * (
        const dp::WindowInfo &
        , const dp::Utf32 &
    )
> GenerateWindow;

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

dp::Window * newWindow(
    const dp::Utf32 &           _TITLE
    , const dp::StringChar *    _DESCRIPTION
    , std::mutex &              _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _closed
    , const GenerateWindow &    _GENERATE_WINDOW
)
{
    dp::Utf32   title;

    if( generateTitle(
        title
        , _TITLE
        , _DESCRIPTION
    ) == false ) {
        return nullptr;
    }

    dp::WindowInfoUnique    infoUnique( dp::newWindowInfo() );
    if( infoUnique.get() == nullptr ) {
        return nullptr;
    }

    auto &  info = *infoUnique;

    dp::setClosingEventHandler(
        info
        , [
            &_mutex
            , &_cond
            , &_closed
        ]
        (
            dp::Window &
        )
        {
            setClose(
                _mutex
                , _cond
                , _closed
            );
        }
    );

    return _GENERATE_WINDOW(
        info
        , title
    );
}

dp::Window * newWindow(
    const dp::Utf32 &           _TITLE
    , const dp::StringChar *    _DESCRIPTION
    , std::mutex &              _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _DESCRIPTION
        , _mutex
        , _cond
        , _closed
        , [](
            const dp::WindowInfo &  _INFO
            , const dp::Utf32 &     _TITLE
        )
        {
            return dp::newWindow(
                _INFO
                , _TITLE
                , WIDTH
                , HEIGHT
            );
        }
    );
}

dp::Window * newWindow(
    const dp::Utf32 &           _TITLE
    , const dp::StringChar *    _DESCRIPTION
    , dp::WindowFlags           _flags
    , std::mutex &              _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _DESCRIPTION
        , _mutex
        , _cond
        , _closed
        , [
            &_flags
        ]
        (
            const dp::WindowInfo &  _INFO
            , const dp::Utf32 &     _TITLE
        )
        {
            return dp::newWindow(
                _INFO
                , _TITLE
                , WIDTH
                , HEIGHT
                , _flags
            );
        }
    );
}

dp::Window * newWindowWithPosition(
    const dp::Utf32 &           _TITLE
    , const dp::StringChar *    _DESCRIPTION
    , std::mutex &              _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _DESCRIPTION
        , _mutex
        , _cond
        , _closed
        , [](
            const dp::WindowInfo &  _INFO
            , const dp::Utf32 &     _TITLE
        )
        {
            return dp::newWindow(
                _INFO
                , _TITLE
                , X
                , Y
                , WIDTH
                , HEIGHT
            );
        }
    );
}

dp::Window * newWindowWithPosition(
    const dp::Utf32 &           _TITLE
    , const dp::StringChar *    _DESCRIPTION
    , dp::WindowFlags           _flags
    , std::mutex &              _mutex
    , std::condition_variable & _cond
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _DESCRIPTION
        , _mutex
        , _cond
        , _closed
        , [
            &_flags
        ]
        (
            const dp::WindowInfo &  _INFO
            , const dp::Utf32 &     _TITLE
        )
        {
            return dp::newWindow(
                _INFO
                , _TITLE
                , X
                , Y
                , WIDTH
                , HEIGHT
                , _flags
            );
        }
    );
}

struct ThreadProc
{
private:
    std::mutex &                mutex;
    std::condition_variable &   cond;
    const NewWindow &           NEW_WINDOW;

public:
    ThreadProc(
        std::mutex &                _mutex
        , std::condition_variable & _cond
        , const NewWindow &         _NEW_WINDOW
    )
        : mutex( _mutex )
        , cond( _cond )
        , NEW_WINDOW( _NEW_WINDOW )
    {
    }

    void operator()(
    )
    {
        dp::Bool    closed = false;

        dp::WindowUnique    windowUnique(
            this->NEW_WINDOW(
                this->mutex
                , this->cond
                , closed
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

    std::thread noneFlags(
        ThreadProc(
            mutex
            , cond
            , [
                &title
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , "none WindowFlags"
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    noneFlagsJoiner( &noneFlags );

    std::thread plain(
        ThreadProc(
            mutex
            , cond
            , [
                &title
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , "PLAIN"
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    plainJoiner( &plain );

    std::thread unresizable(
        ThreadProc(
            mutex
            , cond
            , [
                &title
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , "UNRESIZABLE"
                    , dp::WindowFlags::UNRESIZABLE
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    unresizableJoiner( &unresizable );

    std::thread alwaysOnTop(
        ThreadProc(
            mutex
            , cond
            , [
                &title
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , "ALWAYS_ON_TOP"
                    , dp::WindowFlags::ALWAYS_ON_TOP
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    alwaysOnTopJoiner( &alwaysOnTop );

    std::thread noneFlagsWithPosition(
        ThreadProc(
            mutex
            , cond
            , [
                &title
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindowWithPosition(
                    title
                    , "none WindowFlags with position"
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    noneFlagsWithPositionJoiner( &noneFlagsWithPosition );

    std::thread unresizableWithPosition(
        ThreadProc(
            mutex
            , cond
            , [
                &title
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindowWithPosition(
                    title
                    , "UNRESIZABLE with position"
                    , dp::WindowFlags::UNRESIZABLE
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    unresizableWithPositionJoiner( &unresizableWithPosition );

    return 0;
}
