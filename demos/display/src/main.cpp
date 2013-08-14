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

dp::Bool showDisplay(
    const dp::Display &         _DISPLAY
    , const dp::DisplayMode &   _MODE
)
{
    const auto  ROTATE = dp::displayGetRotate( _DISPLAY );

    const auto  ROTATE_STRING = getDisplayRotateString( ROTATE );
    if( ROTATE_STRING == nullptr ) {
        return false;
    }

    std::printf(
        "%dx%d+%d+%d ( Mode : %dx%d %fHz, Rotate : %s )\n"
        , dp::displayGetWidth( _DISPLAY )
        , dp::displayGetHeight( _DISPLAY )
        , dp::displayGetX( _DISPLAY )
        , dp::displayGetY( _DISPLAY )
        , dp::displayModeGetWidth( _MODE )
        , dp::displayModeGetHeight( _MODE )
        , dp::displayModeGetRefreshRate( _MODE )
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

        dp::DisplayUnique   displayUnique( dp::displayNewFromKey( KEY ) );
        if( displayUnique.get() == nullptr ) {
            continue;
        }

        const auto &    DISPLAY = *displayUnique;

        const auto &    MODE_KEY = dp::displayGetModeKey(
            DISPLAY
        );

        dp::DisplayModeUnique   modeUnique( dp::displayModeNew( MODE_KEY ) );
        if( modeUnique.get() == nullptr ) {
            continue;
        }

        const auto &    MODE = *modeUnique;

        showDisplay(
            DISPLAY
            , MODE
        );
    }
}

void showDisplaysWithIndex(
    //TODO
)
{
    //TODO
}

void configDisplayMenu(
    //TODO
)
{
    while( 1 ) {
        std::printf( "config display\n" );
        showDisplaysWithIndex(
            //TODO
        );
        std::printf( "\n" );
        std::printf( "* : cancel\n" );

        dp::Int index;
        if( inputInt( index ) == false ) {
            return;
        }

        //TODO
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

        dp::Int input;
        if( inputInt( input ) == false ) {
            return;
        }

        switch( input ) {
        case 0:
            showDisplays(
                _mutex
                , _KEY_UNIQUES
            );
            break;

        case 1:
            configDisplayMenu(
                //TODO
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
            return dp::displayKeyEquals(
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

    dp::DisplayManagerInfoUnique    managerInfoUnique( dp::displayManagerInfoNew() );

    auto &  managerInfo = *managerInfoUnique;

    dp::displayManagerInfoSetConnectEventHandler(
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

    dp::DisplayManagerUnique    managerUnique( dp::displayManagerNew( managerInfo ) );

    mainMenu(
        mutex
        , keyUniques
    );

    return 0;
}
