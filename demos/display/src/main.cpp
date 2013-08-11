#include "dp/cli.h"
#include "dp/display/displaymanager.h"
#include "dp/display/displaykey.h"

#include <cstdio>

void connectDisplay(
)
{
    //TODO
    std::printf( "connected\n" );
}

void disconnectDisplay(
)
{
    //TODO
    std::printf( "disconnected\n" );
}

dp::Int dpMain(
    dp::Args &
)
{
    dp::DisplayManagerInfoUnique    managerInfoUnique( dp::displayManagerInfoNew() );

    auto &  managerInfo = *managerInfoUnique;

    dp::displayManagerInfoSetConnectEventHandler(
        managerInfo
        , [](
            dp::DisplayManager &        _manager
            , dp::DisplayKeyUnique &&   _keyUnique
            , dp::Bool                  _connected
        )
        {
            if( _connected ) {
                connectDisplay();
            } else {
                disconnectDisplay();
            }
        }
    );

    dp::DisplayManagerUnique    managerUnique( dp::displayManagerNew( managerInfo ) );

    std::printf( "Press ENTER to quit\n" );

    std::getchar();

    return 0;
}
