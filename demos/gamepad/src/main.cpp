#include "dp/cli.h"
#include "dp/input/gamepadmanager.h"
#include "dp/input/gamepadkey.h"
#include "dp/input/gamepad.h"
#include "dp/common/stringconverter.h"

#include <map>
#include <cstdio>
#include <algorithm>

typedef std::map<
    dp::GamePadKeyUnique
    , dp::GamePadUnique
    , dp::GamePadKeyLess< dp::GamePadKeyUnique >
> GamePadUniques;

struct FindGamePadUnique
{
private:
    const dp::GamePadKey &  KEY;

public:
    FindGamePadUnique(
        const dp::GamePadKey &  _KEY
    )
        : KEY( _KEY )
    {
    }

    dp::Bool operator()(
        const GamePadUniques::value_type &  _VALUE
    ) const
    {
        return dp::gamePadKeyEquals(
            this->KEY
            , *( _VALUE.first )
        );
    }
};

void connectGamePad(
    GamePadUniques &            _gamePadUniques
    , dp::GamePadKeyUnique &&   _keyUnique
    , const dp::GamePadInfo &   _INFO
)
{
    const auto  END = _gamePadUniques.end();
    if( std::find_if(
        _gamePadUniques.begin()
        , END
        , FindGamePadUnique( *_keyUnique )
    ) != END ) {
        return;
    }

    dp::GamePadUnique   gamePadUnique(
        dp::gamePadNew(
            *_keyUnique
            , _INFO
        )
    );
    if( gamePadUnique.get() == nullptr ) {
        return;
    }

    auto &  gamePad = *gamePadUnique;

    dp::String  name;
    dp::ULong   buttons;
    dp::ULong   axes;

    dp::Utf32   nameUtf32;

    if( dp::gamePadGetName(
        gamePad
        , nameUtf32
    ) == false ) {
        return;
    }

    if( dp::gamePadGetButtons(
        gamePad
        , buttons
    ) == false ) {
        return;
    }

    if( dp::gamePadGetAxes(
        gamePad
        , axes
    ) == false ) {
        return;
    }

    if( dp::toString(
        name
        , nameUtf32
    ) == false ) {
        return;
    }

    _gamePadUniques.emplace(
        std::move( _keyUnique )
        , std::move( gamePadUnique )
    );

    std::printf( "gamepad connected\n" );
    std::printf( "  name : %s\n", name.c_str() );
    std::printf( "  buttons : %llu\n", buttons );
    std::printf( "  axes : %llu\n", axes );
}

void disconnectGamePad(
    GamePadUniques &                _gamePadUniques
    , const dp::GamePadKeyUnique &  _KEY_UNIQUE
)
{
    const auto  END = _gamePadUniques.end();

    auto    it = std::find_if(
        _gamePadUniques.begin()
        , END
        , FindGamePadUnique( *_KEY_UNIQUE )
    );
    if( it == END ) {
        return;
    }

    _gamePadUniques.erase( it );

    std::printf( "gamepad disconnected\n" );
}

dp::Int dpMain(
    dp::Args &
)
{
    GamePadUniques  gamePadUniques;

    dp::GamePadInfoUnique   infoUnique( dp::gamePadInfoNew() );

    auto &  info = *infoUnique;

    dp::gamePadInfoSetButtonEventHandler(
        info
        , [](
            dp::GamePad &   _gamePad
            , dp::ULong     _index
            , dp::Bool      _pressed
        )
        {
            std::printf(
                "gamepad button[%llu] %s\n"
                , _index
                , _pressed
                    ? "press"
                    : "release"
            );
        }
    );
    dp::gamePadInfoSetAxisEventHandler(
        info
        , [](
            dp::GamePad &   _gamePad
            , dp::ULong     _index
            , dp::Long      _value
        )
        {
            std::printf(
                "gamepad axis[%llu] value : %lld\n"
                , _index
                , _value
            );
        }
    );

    dp::GamePadManagerInfoUnique    managerInfoUnique( dp::gamePadManagerInfoNew() );

    auto &  managerInfo = *managerInfoUnique;

    dp::gamePadManagerInfoSetConnectEventHandler(
        managerInfo
        , [
            &gamePadUniques
            , &info
        ]
        (
            dp::GamePadManager &        _manager
            , dp::GamePadKeyUnique &&   _keyUnique
            , dp::Bool                  _connected
        )
        {
            if( _connected ) {
                connectGamePad(
                    gamePadUniques
                    , std::move( _keyUnique )
                    , info
                );
            } else {
                disconnectGamePad(
                    gamePadUniques
                    , _keyUnique
                );
            }
        }
    );

    dp::GamePadManagerUnique    managerUnique( dp::gamePadManagerNew( managerInfo ) );

    std::printf( "Press ENTER to quit\n" );

    std::getchar();

    return 0;
}
