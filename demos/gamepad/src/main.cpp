#include "dp/cli.h"
#include "dp/input/gamepadmanager.h"
#include "dp/input/gamepadkey.h"
#include "dp/input/gamepad.h"
#include "dp/common/stringconverter.h"
#include "dp/common/functional.h"

#include <map>
#include <cstdio>
#include <algorithm>

typedef std::map<
    dp::GamePadKeyUnique
    , dp::GamePadUnique
    , dp::Less< dp::GamePadKeyUnique >
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
        return dp::equals(
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

    auto    gamePadUnique = dp::unique(
        dp::newGamePad(
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

    if( dp::getName(
        gamePad
        , nameUtf32
    ) == false ) {
        return;
    }

    if( dp::getButtons(
        gamePad
        , buttons
    ) == false ) {
        return;
    }

    if( dp::getAxes(
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

    auto    infoUnique = dp::unique( dp::newGamePadInfo() );
    if( infoUnique.get() == nullptr ) {
        std::printf( "dp::GamePadInfoの生成に失敗\n" );

        return 1;
    }

    auto &  info = *infoUnique;

    dp::setButtonEventHandler(
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
    dp::setAxisEventHandler(
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

    auto    managerInfoUnique = dp::unique( dp::newGamePadManagerInfo() );
    if( managerInfoUnique.get() == nullptr ) {
        std::printf( "dp::GamePadManagerInfoの生成に失敗\n" );

        return 1;
    }

    auto &  managerInfo = *managerInfoUnique;

    dp::setConnectEventHandler(
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

    auto    managerUnique = dp::unique( dp::newGamePadManager( managerInfo ) );
    if( managerUnique.get() == nullptr ) {
        std::printf( "dp::GamePadManagerの生成に失敗\n" );

        return 1;
    }

    std::printf( "Press ENTER to quit\n" );

    std::getchar();

    return 0;
}
