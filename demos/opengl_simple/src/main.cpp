#include "dp/gui.h"
#include "dp/common/primitives.h"
#include "dp/common/stringconverter.h"
#include "dp/window/window.h"
#include "dp/opengl/glcontext.h"
#include "dp/opengl/gl.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <cstdio>

const auto  TITLE_STRING = "OpenGL simple";
const auto  WIDTH = 100;
const auto  HEIGHT = 100;

dp::GLContext * newGLContext(
)
{
    dp::GLContextInfoUnique infoUnique( dp::newGLContextInfo() );
    if( infoUnique.get() == nullptr ) {
        std::printf( "dp::GLContextInfoの生成に失敗\n" );

        return nullptr;
    }
    auto &  info = *infoUnique;

    return dp::newGLContext( info );
}

dp::Bool loadGLProcs(
)
{
    const auto  GL_PROC_PTRS = {
        dp::toGLProcPtr( dp::glEnable ),
        dp::toGLProcPtr( dp::glDepthFunc ),
        dp::toGLProcPtr( dp::glClearColor ),
        dp::toGLProcPtr( dp::glViewport ),
        dp::toGLProcPtr( dp::glMatrixMode ),
        dp::toGLProcPtr( dp::glLoadIdentity ),
        dp::toGLProcPtr( dp::glFrustum ),
        dp::toGLProcPtr( dp::glTranslatef ),
        dp::toGLProcPtr( dp::glScalef ),
        dp::toGLProcPtr( dp::glRotatef ),
        dp::toGLProcPtr( dp::glClear ),
        dp::toGLProcPtr( dp::glBegin ),
        dp::toGLProcPtr( dp::glEnd ),
        dp::toGLProcPtr( dp::glColor3f ),
        dp::toGLProcPtr( dp::glVertex3f ),
    };

    return dp::loadGLProcs( GL_PROC_PTRS );
}

void rotate(
    std::mutex &        _mutex
    , const dp::Float & _ROTATE_X
    , const dp::Float & _ROTATE_Y
    , const dp::Float & _ROTATE_Z
)
{
    dp::glLoadIdentity();

    dp::Float   rotateX;
    dp::Float   rotateY;
    dp::Float   rotateZ;
    {
        std::unique_lock< std::mutex >  lock( _mutex );

        rotateX = _ROTATE_X;
        rotateY = _ROTATE_Y;
        rotateZ = _ROTATE_Z;
    }

    dp::glRotatef(
        rotateX
        , 1
        , 0
        , 0
    );

    dp::glRotatef(
        rotateY
        , 0
        , 1
        , 0
    );

    dp::glRotatef(
        rotateZ
        , 0
        , 0
        , 1
    );
}

dp::Window * newWindow(
    dp::GLContext &             _glContext
    , std::mutex &              _mutexForEnded
    , std::condition_variable & _condForEnded
    , dp::Bool &                _ended
    , std::mutex &              _mutexForRotate
    , const dp::Float &         _ROTATE_X
    , const dp::Float &         _ROTATE_Y
    , const dp::Float &         _ROTATE_Z
)
{
    dp::Utf32   title;
    if( dp::toUtf32(
        title
        , TITLE_STRING
    ) == false ) {
        std::printf( "ウィンドウタイトルの文字コード変換に失敗\n" );

        return nullptr;
    }

    dp::WindowInfoUnique    infoUnique( dp::newWindowInfo() );
    if( infoUnique.get() == nullptr ) {
        std::printf( "dp::WindowInfoの生成に失敗\n" );

        return nullptr;
    }
    auto &  info = *infoUnique;

    dp::setCloseEventHandler(
        info
        , [
            &_mutexForEnded
            , &_condForEnded
            , &_ended
        ]
        (
            dp::Window &
        )
        {
            std::unique_lock< std::mutex >  lock( _mutexForEnded );

            _ended = true;

            _condForEnded.notify_one();
        }
    );

    dp::setBeginPaintEventHandler(
        info
        , [
            &_glContext
        ]
        (
            dp::Window &    _window
        )
        {
            dp::glMakeCurrent(
                _window
                , _glContext
                , true
            );

            dp::glEnable( dp::GL_DEPTH_TEST );
            dp::glDepthFunc( dp::GL_LEQUAL );

            dp::glClearColor(
                0
                , 0
                , 0
                , 0
            );

            dp::glMatrixMode( dp::GL_PROJECTION );

            dp::glLoadIdentity();
            dp::glFrustum(
                -0.5
                , 0.5
                , -0.5
                , 0.5
                , 1
                , 10
            );
            dp::glTranslatef(
                0
                , 0
                , -5
            );

            dp::glMatrixMode( dp::GL_MODELVIEW );
        }
    );

    dp::setEndPaintEventHandler(
        info
        , [](
            dp::Window &
        )
        {
            dp::glMakeCurrent();
        }
    );

    dp::setSizeEventHandler(
        info
        , [](
            dp::Window &
            , dp::Int       _width
            , dp::Int       _height
        )
        {
            dp::glViewport(
                0
                , 0
                , _width
                , _height
            );
        }
    );

    dp::setPaintEventHandler(
        info
        , [
            &_mutexForRotate
            , &_ROTATE_X
            , &_ROTATE_Y
            , &_ROTATE_Z
        ]
        (
            dp::Window &    _window
            , dp::Int
            , dp::Int
            , dp::Int
            , dp::Int
        )
        {
            dp::glClear(
                dp::GL_COLOR_BUFFER_BIT |
                dp::GL_DEPTH_BUFFER_BIT
            );

            const dp::Int   VERTICES[][ 3 ] = {
                { -1,  1,  1 },
                { -1, -1,  1 },
                {  1, -1,  1 },

                { -1,  1,  1 },
                {  1, -1,  1 },
                {  1,  1,  1 },


                { -1,  1, -1 },
                { -1,  1,  1 },
                {  1,  1,  1 },

                { -1,  1, -1 },
                {  1,  1,  1 },
                {  1,  1, -1 },


                { -1, -1, -1 },
                { -1,  1, -1 },
                {  1,  1, -1 },

                { -1, -1, -1 },
                {  1,  1, -1 },
                {  1, -1, -1 },


                { -1, -1,  1 },
                { -1, -1, -1 },
                {  1, -1, -1 },

                { -1, -1,  1 },
                {  1, -1, -1 },
                {  1, -1,  1 },


                {  1,  1,  1 },
                {  1, -1,  1 },
                {  1, -1, -1 },

                {  1,  1,  1 },
                {  1, -1, -1 },
                {  1,  1, -1 },


                { -1,  1, -1 },
                { -1, -1, -1 },
                { -1, -1,  1 },

                { -1,  1, -1 },
                { -1, -1,  1 },
                { -1,  1,  1 },
            };

            rotate(
                _mutexForRotate
                , _ROTATE_X
                , _ROTATE_Y
                , _ROTATE_Z
            );

            dp::glBegin( dp::GL_TRIANGLES );

            for( const auto & VERTEX : VERTICES ) {
                dp::glColor3f(
                    VERTEX[ 0 ] >= 0 ? VERTEX[ 0 ] : 0
                    , VERTEX[ 1 ] >= 0 ? VERTEX[ 1 ] : 0
                    , VERTEX[ 2 ] >= 0 ? VERTEX[ 2 ] : 0
                );

                dp::glVertex3f(
                    VERTEX[ 0 ]
                    , VERTEX[ 1 ]
                    , VERTEX[ 2 ]
                );
            }

            dp::glEnd();

            dp::glSwapBuffers( _window );
        }
    );

    return dp::newWindow(
        info
        , title
        , WIDTH
        , HEIGHT
        , dp::WindowFlags::UNRESIZABLE
    );
}

void waitEnd(
    std::mutex &                _mutex
    , std::condition_variable & _cond
    , const dp::Bool          & _ENDED
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
    dp::Args &
)
{
    dp::GLContextUnique glContextUnique( newGLContext() );
    if( glContextUnique.get() == nullptr ) {
        std::printf( "OpenGLコンテキスト生成に失敗\n" );

        return 1;
    }
    auto &  glContext = *glContextUnique;

    if( loadGLProcs() == false ) {
        std::printf( "GL関数のロードに失敗\n" );

        return 1;
    }

    std::mutex              mutexForEnded;
    std::condition_variable condForEnded;
    dp::Bool                ended = false;

    std::mutex  mutexForRotate;
    dp::Float   rotateX = 0;
    dp::Float   rotateY = 0;
    dp::Float   rotateZ = 0;

    dp::WindowUnique    windowUnique(
        newWindow(
            glContext
            , mutexForEnded
            , condForEnded
            , ended
            , mutexForRotate
            , rotateX
            , rotateY
            , rotateZ
        )
    );
    if( windowUnique.get() == nullptr ) {
        std::printf( "ウィンドウ生成に失敗\n" );

        return 1;
    }
    auto &  window = *windowUnique;

    std::condition_variable condForRotate;
    std::thread timerThread(
        [
            &ended
            , &mutexForRotate
            , &condForRotate
            , &rotateX
            , &rotateY
            , &rotateZ
            , &window
        ]
        {
            while( ended == false ) {
                std::unique_lock< std::mutex >  lock( mutexForRotate );

                rotateX += 0.03;
                if( rotateX > 360 ) {
                    rotateX -= 360;
                }

                rotateY += 0.05;
                if( rotateY > 360 ) {
                    rotateY -= 360;
                }

                rotateZ += 0.07;
                if( rotateZ > 360 ) {
                    rotateZ -= 360;
                }

                dp::repaint(
                    window
                    , 0
                    , 0
                    , 100
                    , 100
                );

                condForRotate.wait_for(
                    lock
                    , std::chrono::milliseconds( 1 )
                );
            }
        }
    );

    waitEnd(
        mutexForEnded
        , condForEnded
        , ended
    );

    timerThread.join();

    return 0;
}
