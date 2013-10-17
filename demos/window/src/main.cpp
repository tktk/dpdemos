#include "dp/cli.h"
#include "dp/window/window.h"
#include "dp/window/windowflags.h"
#include "dp/common/stringconverter.h"
#include "dp/common/thread.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <cstdio>

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

struct Bounds
{
    dp::Bool    initializePosition;
    dp::Int     x;
    dp::Int     y;

    dp::Bool    initializeSize;
    dp::Int     width;
    dp::Int     height;

    Bounds(
    )
        : initializePosition( false )
        , initializeSize( false )
    {
    }
};

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

void setTitle(
    dp::Window &            _window
    , const Bounds &        _BOUNDS
    , const dp::String &    _TITLE_STRING
)
{
    if( _BOUNDS.initializePosition == false || _BOUNDS.initializeSize == false ) {
        return;
    }

    std::ostringstream  stream;

    stream << _TITLE_STRING << ' ' << _BOUNDS.width << 'x' << _BOUNDS.height << '+' << _BOUNDS.x << '+' << _BOUNDS.y;

    dp::Utf32   title;
    dp::toUtf32(
        title
        , stream.str()
    );

    dp::setTitle(
        _window
        , title
    );
}

dp::Window * newWindow(
    const dp::Utf32 &           _TITLE
    , std::mutex &              _mutexForBounds
    , Bounds &                  _bounds
    , const dp::StringChar *    _DESCRIPTION
    , std::mutex &              _mutexForClosed
    , std::condition_variable & _condForClosed
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

    dp::String  titleString;

    if( dp::toString(
        titleString
        , title
    ) == false ) {
        return nullptr;
    }

    auto    infoUnique = dp::unique( dp::newWindowInfo() );
    if( infoUnique.get() == nullptr ) {
        return nullptr;
    }

    auto &  info = *infoUnique;

    dp::setCloseEventHandler(
        info
        , [
            &_mutexForClosed
            , &_condForClosed
            , &_closed
        ]
        (
            dp::Window &
        )
        {
            setClose(
                _mutexForClosed
                , _condForClosed
                , _closed
            );
        }
    );

    dp::setPositionEventHandler(
        info
        , [
            &_mutexForBounds
            , &_bounds
            , titleString
        ]
        (
            dp::Window &    _window
            , dp::Int       _x
            , dp::Int       _y
        )
        {
            std::unique_lock< std::mutex >  lock( _mutexForBounds );

            _bounds.initializePosition = true;
            _bounds.x = _x;
            _bounds.y = _y;

            setTitle(
                _window
                , _bounds
                , titleString
            );
        }
    );

    dp::setSizeEventHandler(
        info
        , [
            &_mutexForBounds
            , &_bounds
            , titleString
        ]
        (
            dp::Window &    _window
            , dp::Int       _width
            , dp::Int       _height
        )
        {
            std::unique_lock< std::mutex >  lock( _mutexForBounds );

            _bounds.initializeSize = true;
            _bounds.width = _width;
            _bounds.height = _height;

            setTitle(
                _window
                , _bounds
                , titleString
            );
        }
    );

    dp::setBeginPaintEventHandler(
        info
        , [
            titleString
        ]
        (
            dp::Window &
        )
        {
            std::printf(
                "Begin paint [%s]\n"
                , titleString.c_str()
            );
        }
    );

    dp::setEndPaintEventHandler(
        info
        , [
            titleString
        ]
        (
            dp::Window &
        )
        {
            std::printf(
                "End paint [%s]\n"
                , titleString.c_str()
            );
        }
    );

    dp::setPaintEventHandler(
        info
        , [
            titleString
        ]
        (
            dp::Window &    _window
            , dp::Int       _x
            , dp::Int       _y
            , dp::Int       _width
            , dp::Int       _height
        )
        {
            std::printf(
                "paint %dx%d+%d+%d [%s]\n"
                , _width
                , _height
                , _x
                , _y
                , titleString.c_str()
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
    , std::mutex &              _mutexForBounds
    , Bounds &                  _bounds
    , const dp::StringChar *    _DESCRIPTION
    , std::mutex &              _mutexForClosed
    , std::condition_variable & _condForClosed
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _mutexForBounds
        , _bounds
        , _DESCRIPTION
        , _mutexForClosed
        , _condForClosed
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
    , std::mutex &              _mutexForBounds
    , Bounds &                  _bounds
    , const dp::StringChar *    _DESCRIPTION
    , dp::WindowFlags           _flags
    , std::mutex &              _mutexForClosed
    , std::condition_variable & _condForClosed
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _mutexForBounds
        , _bounds
        , _DESCRIPTION
        , _mutexForClosed
        , _condForClosed
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
    , std::mutex &              _mutexForBounds
    , Bounds &                  _bounds
    , const dp::StringChar *    _DESCRIPTION
    , std::mutex &              _mutexForClosed
    , std::condition_variable & _condForClosed
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _mutexForBounds
        , _bounds
        , _DESCRIPTION
        , _mutexForClosed
        , _condForClosed
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
    , std::mutex &              _mutexForBounds
    , Bounds &                  _bounds
    , const dp::StringChar *    _DESCRIPTION
    , dp::WindowFlags           _flags
    , std::mutex &              _mutexForClosed
    , std::condition_variable & _condForClosed
    , dp::Bool &                _closed
)
{
    return newWindow(
        _TITLE
        , _mutexForBounds
        , _bounds
        , _DESCRIPTION
        , _mutexForClosed
        , _condForClosed
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
    NewWindow                   newWindow;

public:
    ThreadProc(
        std::mutex &                _mutex
        , std::condition_variable & _cond
        , const NewWindow &         _NEW_WINDOW
    )
        : mutex( _mutex )
        , cond( _cond )
        , newWindow( _NEW_WINDOW )
    {
    }

    void operator()(
    )
    {
        dp::Bool    closed = false;

        auto    windowUnique = dp::unique(
            this->newWindow(
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

    std::mutex  mutexForNoneFlagsBounds;
    Bounds      noneFlagsBounds;
    std::thread noneFlags(
        ThreadProc(
            mutex
            , cond
            , [
                &title
                , &mutexForNoneFlagsBounds
                , &noneFlagsBounds
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , mutexForNoneFlagsBounds
                    , noneFlagsBounds
                    , "none WindowFlags"
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    noneFlagsJoiner( &noneFlags );

    std::mutex  mutexForPlainBounds;
    Bounds      plainBounds;
    std::thread plain(
        ThreadProc(
            mutex
            , cond
            , [
                &title
                , &mutexForPlainBounds
                , &plainBounds
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , mutexForPlainBounds
                    , plainBounds
                    , "PLAIN"
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    plainJoiner( &plain );

    std::mutex  mutexForUnresizableBounds;
    Bounds      unresizableBounds;
    std::thread unresizable(
        ThreadProc(
            mutex
            , cond
            , [
                &title
                , &mutexForUnresizableBounds
                , &unresizableBounds
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , mutexForUnresizableBounds
                    , unresizableBounds
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

    std::mutex  mutexForAlwaysOnTopBounds;
    Bounds      alwaysOnTopBounds;
    std::thread alwaysOnTop(
        ThreadProc(
            mutex
            , cond
            , [
                &title
                , &mutexForAlwaysOnTopBounds
                , &alwaysOnTopBounds
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindow(
                    title
                    , mutexForAlwaysOnTopBounds
                    , alwaysOnTopBounds
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

    std::mutex  mutexForNoneFlagsWithPositionBounds;
    Bounds      noneFlagsWithPositionBounds;
    std::thread noneFlagsWithPosition(
        ThreadProc(
            mutex
            , cond
            , [
                &title
                , &mutexForNoneFlagsWithPositionBounds
                , &noneFlagsWithPositionBounds
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindowWithPosition(
                    title
                    , mutexForNoneFlagsWithPositionBounds
                    , noneFlagsWithPositionBounds
                    , "none WindowFlags with position"
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    noneFlagsWithPositionJoiner( &noneFlagsWithPosition );

    std::mutex  mutexForMultiFlagsWithPositionBounds;
    Bounds      multiFlagsWithPositionBounds;
    std::thread multiFlagsWithPosition(
        ThreadProc(
            mutex
            , cond
            , [
                &title
                , &mutexForMultiFlagsWithPositionBounds
                , &multiFlagsWithPositionBounds
            ]
            (
                std::mutex &                _mutex
                , std::condition_variable & _cond
                , dp::Bool &                _closed
            )
            {
                return newWindowWithPosition(
                    title
                    , mutexForMultiFlagsWithPositionBounds
                    , multiFlagsWithPositionBounds
                    , "UNRESIZABLE | ALWAYS_ON_TOP with position"
                    , dp::WindowFlags::UNRESIZABLE | dp::WindowFlags::ALWAYS_ON_TOP
                    , _mutex
                    , _cond
                    , _closed
                );
            }
        )
    );
    dp::ThreadJoiner    multiFlagsWithPositionJoiner( &multiFlagsWithPosition );

    return 0;
}
