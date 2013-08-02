#include "dp/cli.h"
#include "dp/input/gamepadmanager.h"
#include "dp/input/gamepadkey.h"

#include <cstdio>

dp::Int dpMain(
    dp::Args &
)
{
    dp::GamePadManagerInfoPtr   managerInfoPtr( dp::gamePadManagerInfoNew() );

    auto &  managerInfo = *managerInfoPtr;

    dp::gamePadManagerInfoSetConnectEventHandler(
        managerInfo
        , [](
            dp::GamePadManager &        _manager
            , const dp::GamePadKey &    _KEY
            , dp::Bool                  _connected
        )
        {
            if( _connected ) {
                std::printf( "connected\n" );
            } else {
                std::printf( "disconnected\n" );
            }
        }
    );

    dp::GamePadManagerPtr   managerPtr( dp::gamePadManagerNew( managerInfo ) );

    std::printf( "Press ENTER to quit\n" );

    std::getchar();

    return 0;
}
