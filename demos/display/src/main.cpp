#include "dp/cli.h"
#include "dp/display/displaymanager.h"
#include "dp/display/displaykey.h"
#include "dp/display/display.h"
#include "dp/display/displayrotate.h"
#include "dp/display/displaymode.h"
#include "dp/common/primitives.h"

#include "input.h"

#include <mutex>
#include <utility>
#include <algorithm>
#include <cstdio>

typedef std::vector< dp::DisplayKeyUnique > DisplayKeyUniques;

const dp::StringChar * getDisplayRotateString(
    dp::DisplayRotate   _rotate
)
{
    switch( _rotate ) {
    case dp::DisplayRotate::NORMAL:
        return "normal";
        break;

    case dp::DisplayRotate::RIGHT:
        return "right";
        break;

    case dp::DisplayRotate::INVERTED:
        return "inverted";
        break;

    case dp::DisplayRotate::LEFT:
        return "left";
        break;

    default:
        break;
    }

    return nullptr;
}

void printDisplayMode(
    const dp::DisplayMode & _MODE
)
{
    std::printf(
        "%dx%d %fHz"
        , dp::getWidth( _MODE )
        , dp::getHeight( _MODE )
        , dp::getRefreshRate( _MODE )
    );
}

void showDisplayMode(
    const dp::DisplayMode & _MODE
)
{
    printDisplayMode(
        _MODE
    );
    std::printf( "\n" );
}

dp::Bool showDisplayMode(
    const dp::DisplayModeKey &  _MODE_KEY
)
{
    auto    modeUnique = dp::unique(
        dp::newDisplayMode(
            _MODE_KEY
        )
    );
    if( modeUnique.get() == nullptr ) {
        return false;
    }

    const auto &    MODE = *modeUnique;

    showDisplayMode(
        MODE
    );

    return true;
}

void showDisplayModesWithIndex(
    const dp::DisplayModeKeyUniques &   _MODE_KEY_UNIQUES
)
{
    auto    index = 0;
    for( const auto & MODE_KEY_UNIQUE : _MODE_KEY_UNIQUES ) {
        const auto &    MODE_KEY = *MODE_KEY_UNIQUE;

        std::printf( "%d : ", index );
        index++;

        if( showDisplayMode(
            MODE_KEY
        ) == false ) {
            std::printf( "missing display mode\n" );
        }
    }
}

dp::Bool showDisplay(
    const dp::DisplayKey &  _KEY
)
{
    auto    displayUnique = dp::unique( dp::newDisplay( _KEY ) );
    if( displayUnique.get() == nullptr ) {
        return false;
    }

    const auto &    DISPLAY = *displayUnique;

    const auto &    MODE_KEY = dp::getModeKey(
        DISPLAY
    );

    auto    modeUnique = dp::unique( dp::newDisplayMode( MODE_KEY ) );
    if( modeUnique.get() == nullptr ) {
        return false;
    }

    const auto &    MODE = *modeUnique;

    const auto  ROTATE = dp::getRotate( DISPLAY );

    const auto  ROTATE_STRING = getDisplayRotateString( ROTATE );
    if( ROTATE_STRING == nullptr ) {
        return false;
    }

    std::printf(
        "%dx%d+%d+%d ( Mode : "
        , dp::getWidth( DISPLAY )
        , dp::getHeight( DISPLAY )
        , dp::getX( DISPLAY )
        , dp::getY( DISPLAY )
    );
    printDisplayMode(
        MODE
    );
    std::printf(
        ", Rotate : %s )\n"
        , ROTATE_STRING
    );

    return true;
}

void showDisplays(
    std::mutex &                _mutex
    , const DisplayKeyUniques & _KEY_UNIQUES
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    for( const auto & KEY_UNIQUE : _KEY_UNIQUES ) {
        const auto &    KEY = *KEY_UNIQUE;

        showDisplay(
            KEY
        );
    }
}

void showDisplaysWithIndex(
    std::mutex &                _mutex
    , const DisplayKeyUniques & _KEY_UNIQUES
)
{
    auto    index = 0;

    std::unique_lock< std::mutex >  lock( _mutex );

    for( const auto & KEY_UNIQUE : _KEY_UNIQUES ) {
        const auto &    KEY = *KEY_UNIQUE;

        std::printf( "%d : ", index );
        index++;

        if( showDisplay(
            KEY
        ) == false ) {
            std::printf( "missing display\n" );
        }
    }
}

void configDisplayInputX(
    dp::Display &   _display
)
{
    std::printf( "input x\n" );

    dp::Int x;

    if( input( x ) == false ) {
        return;
    }

    dp::setX(
        _display
        , x
    );
}

void configDisplayInputY(
    dp::Display &   _display
)
{
    std::printf( "input y\n" );

    dp::Int y;

    if( input( y ) == false ) {
        return;
    }

    dp::setY(
        _display
        , y
    );
}

void configDisplayInputRotate(
    dp::Display &   _display
)
{
    std::printf( "input rotate\n" );
    std::printf( "0 : normal\n" );
    std::printf( "1 : right\n" );
    std::printf( "2 : inverted\n" );
    std::printf( "3 : left\n" );
    std::printf( "\n" );
    std::printf( "* : cancel\n" );

    dp::Int rotateInt;
    if( input( rotateInt ) == false ) {
        return;
    }

    dp::DisplayRotate   rotate;
    switch( rotateInt ) {
    case 0:
        rotate = dp::DisplayRotate::NORMAL;
        break;

    case 1:
        rotate = dp::DisplayRotate::RIGHT;
        break;

    case 2:
        rotate = dp::DisplayRotate::INVERTED;
        break;

    case 3:
        rotate = dp::DisplayRotate::LEFT;
        break;

    default:
        return;
        break;
    }

    dp::setRotate(
        _display
        , rotate
    );
}

void configDisplayInputMode(
    dp::Display &               _display
    , dp::DisplayModeUnique &   _modeUnique
    , const dp::DisplayKey &    _KEY
)
{
    dp::DisplayModeKeyUniques   modeKeyUniques;
    if( dp::enumDisplayModeKeyUniques(
        _KEY
        , modeKeyUniques
    ) == false ) {
        std::printf( "missing display\n" );

        return;
    }

    std::printf( "input mode\n" );
    showDisplayModesWithIndex(
        modeKeyUniques
    );
    std::printf( "\n" );
    std::printf( "* : cancel\n" );

    dp::Int index;
    if( input( index ) == false ) {
        return;
    }

    if( index < 0 || index >= modeKeyUniques.size() ) {
        return;
    }

    auto &  modeKeyUnique = modeKeyUniques[ index ];

    const auto &    MODE_KEY = *modeKeyUnique;

    auto    modeUnique = dp::unique(
        dp::newDisplayMode(
            MODE_KEY
        )
    );
    if( modeUnique.get() == nullptr ) {
        std::printf( "missing display mode\n" );

        return;
    }

    if( dp::setModeKey(
        _display
        , MODE_KEY
    ) == false ) {
        std::printf( "missing display mode\n" );

        return;
    }

    _modeUnique = std::move( modeUnique );
}

void applyDisplay(
    const dp::DisplayKey &      _KEY
    , const dp::Display &       _DISPLAY
    , dp::Int &                 _width
    , dp::Int &                 _height
    , dp::Int &                 _x
    , dp::Int &                 _y
)
{
    if( dp::apply(
        _KEY
        , _DISPLAY
    ) == false ) {
        std::printf( "failed config display\n" );

        return;
    }

    _width = dp::getWidth( _DISPLAY );
    _height = dp::getHeight( _DISPLAY );
    _x = dp::getX( _DISPLAY );
    _y = dp::getY( _DISPLAY );
}

void configDisplay(
    const dp::DisplayKey &  _KEY
)
{
    auto    displayUnique = dp::unique(
        dp::newDisplay(
            _KEY
        )
    );
    if( displayUnique.get() == nullptr ) {
        std::printf( "missing display\n" );

        return;
    }

    auto &  display = *displayUnique;

    auto    displayWidth = dp::getWidth( display );
    auto    displayHeight = dp::getHeight( display );
    auto    displayX = dp::getX( display );
    auto    displayY = dp::getY( display );


    auto    modeUnique = dp::unique(
        dp::newDisplayMode(
            dp::getModeKey(
                display
            )
        )
    );
    if( modeUnique.get() == nullptr ) {
        std::printf( "missing display mode\n" );

        return;
    }

    while( 1 ) {
        const auto  ROTATE_STRING = getDisplayRotateString(
            dp::getRotate( display )
        );
        if( ROTATE_STRING == nullptr ) {
            std::printf( "failed get rotate string\n" );

            return;
        }

        auto &  mode = *modeUnique;

        std::printf(
            "config %dx%d+%d+%d\n"
            , displayWidth
            , displayHeight
            , displayX
            , displayY
        );
        std::printf( "1 : x = %d\n", dp::getX( display ) );
        std::printf( "2 : y = %d\n", dp::getY( display ) );
        std::printf( "3 : rotate = %s\n", ROTATE_STRING );
        std::printf( "4 : mode = " );
        showDisplayMode(
            mode
        );
        std::printf( "\n" );
        std::printf( "0 : apply\n" );
        std::printf( "\n" );
        std::printf( "* : cancel\n" );

        dp::Int inputValue;
        if( input( inputValue ) == false ) {
            return;
        }

        switch( inputValue ) {
        case 1:
            configDisplayInputX(
                display
            );
            break;

        case 2:
            configDisplayInputY(
                display
            );
            break;

        case 3:
            configDisplayInputRotate(
                display
            );
            break;

        case 4:
            configDisplayInputMode(
                display
                , modeUnique
                , _KEY
            );
            break;

        case 0:
            applyDisplay(
                _KEY
                , display
                , displayWidth
                , displayHeight
                , displayX
                , displayY
            );
            break;

        default:
            return;
            break;
        }
    }
}

dp::DisplayKey * cloneDisplay(
    std::mutex &                _mutex
    , const DisplayKeyUniques & _KEY_UNIQUES
    , dp::Int                   _index
)
{
    std::unique_lock< std::mutex >  lock( _mutex );

    if( _index < 0 || _index >= _KEY_UNIQUES.size() ) {
        return nullptr;
    }

    return dp::clone( *( _KEY_UNIQUES[ _index ] ) );
}

void configDisplayMenu(
    std::mutex &                _mutex
    , const DisplayKeyUniques & _KEY_UNIQUES
)
{
    while( 1 ) {
        std::printf( "config display\n" );
        showDisplaysWithIndex(
            _mutex
            , _KEY_UNIQUES
        );
        std::printf( "\n" );
        std::printf( "* : cancel\n" );

        dp::Int index;
        if( input( index ) == false ) {
            return;
        }

        auto    keyUnique = dp::unique(
            cloneDisplay(
                _mutex
                , _KEY_UNIQUES
                , index
            )
        );
        if( keyUnique.get() == nullptr ) {
            return;
        }

        const auto &    KEY = *keyUnique;

        configDisplay(
            KEY
        );
    }
}

void mainMenu(
    std::mutex &                _mutex
    , const DisplayKeyUniques & _KEY_UNIQUES
)
{
    while( 1 ) {
        std::printf( "main menu\n" );
        std::printf( "0 : show displays\n" );
        std::printf( "1 : config display\n" );
        std::printf( "\n" );
        std::printf( "* : quit\n" );

        dp::Int inputValue;
        if( input( inputValue ) == false ) {
            return;
        }

        switch( inputValue ) {
        case 0:
            showDisplays(
                _mutex
                , _KEY_UNIQUES
            );
            break;

        case 1:
            configDisplayMenu(
                _mutex
                , _KEY_UNIQUES
            );
            break;

        default:
            break;
        }
    }
}

DisplayKeyUniques::iterator findDisplayKeyUnique(
    DisplayKeyUniques &             _keyUniques
    , const dp::DisplayKeyUnique &  _KEY_UNIQUE
)
{
    return std::find_if(
        _keyUniques.begin()
        , _keyUniques.end()
        , [
            &_KEY_UNIQUE
        ]
        (
            const dp::DisplayKeyUnique &    _KEY_UNIQUE2
        )
        {
            return dp::equals(
                *_KEY_UNIQUE
                , *_KEY_UNIQUE2
            );
        }
    );
}

void addDisplayKeyUnique(
    DisplayKeyUniques &         _keyUniques
    , dp::DisplayKeyUnique &&   _keyUnique
)
{
    if( findDisplayKeyUnique(
        _keyUniques
        , _keyUnique
    ) != _keyUniques.end() ) {
        return;
    }

    _keyUniques.push_back( std::move( _keyUnique ) );
}

void removeDisplayKeyUnique(
    DisplayKeyUniques &             _keyUniques
    , const dp::DisplayKeyUnique &  _KEY_UNIQUE
)
{
    auto    it = findDisplayKeyUnique(
        _keyUniques
        , _KEY_UNIQUE
    );
    if( it == _keyUniques.end() ) {
        return;
    }

    _keyUniques.erase( it );
}

dp::Int dpMain(
    dp::Args &
)
{
    std::mutex          mutex;
    DisplayKeyUniques   keyUniques;

    auto    managerInfoUnique = dp::unique( dp::newDisplayManagerInfo() );
    if( managerInfoUnique.get() == nullptr ) {
        std::printf( "dp::DisplayManagerInfoの生成に失敗\n" );

        return 1;
    }

    auto &  managerInfo = *managerInfoUnique;

    dp::setConnectEventHandler(
        managerInfo
        , [
            &mutex
            , &keyUniques
        ]
        (
            dp::DisplayManager &        _manager
            , dp::DisplayKeyUnique &&   _keyUnique
            , dp::Bool                  _connected
        )
        {
            std::unique_lock< std::mutex >  lock( mutex );

            if( _connected ) {
                addDisplayKeyUnique(
                    keyUniques
                    , std::move( _keyUnique )
                );
            } else {
                removeDisplayKeyUnique(
                    keyUniques
                    , _keyUnique
                );
            }
        }
    );

    auto    managerUnique = dp::unique( dp::newDisplayManager( managerInfo ) );
    if( managerUnique.get() == nullptr ) {
        std::printf( "dp::DisplayManagerの生成に失敗\n" );

        return 1;
    }

    mainMenu(
        mutex
        , keyUniques
    );

    return 0;
}
