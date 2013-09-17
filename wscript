# -*- coding: utf-8 -*-

from wscripts import common

import os.path

APPNAME = common.APPNAME
VERSION = '0.5.0'

out = common.BUILD_DIR

PLATFORMS = {
    common.X86 : [
        common.LINUX,
        common.WINDOWS,
    ],
    common.AMD64 : [
        common.LINUX,
        common.WINDOWS,
    ],
}

BUILD_TYPES = [
    common.DEBUG,
    common.RELEASE,
]

CXXFLAGS = {
    common.LINUX : {
        common.COMMON : [
            '-fno-rtti',
            '-fvisibility=hidden',
            '-std=c++0x',
            '-Wall',
            '-DLINUX',
        ],
        common.DEBUG : [
            '-O0',
            '-g',
            '-DDEBUG',
        ],
        common.RELEASE : [
            '-O2',
        ],
    },
    common.WINDOWS : {
        common.COMMON : [
            '/EHs',
            '/D_VARIADIC_MAX=6',    # vs2012用
            '/Wall',
            '/nologo',
            '/DWINDOWS',
        ],
        common.DEBUG : [
            '/MDd',
            '/Od',
            '/DDEBUG',
        ],
        common.RELEASE : [
            '/MD',
            '/O2',
            '/Oi',
            '/GL',
        ],
    },
}

LINKFLAGS = {
    common.LINUX : {
        common.COMMON : [
        ],
        common.DEBUG : [
            '-rdynamic',
        ],
        common.RELEASE : [
        ],
    },
    common.WINDOWS : {
        common.COMMON : [
            '/NOLOGO',
            '/DYNAMICBASE',
            '/NXCOMPAT',
        ],
        common.DEBUG : [
        ],
        common.RELEASE : [
            '/OPT:REF',
            '/OPT:ICF',
            '/LTCG',
        ],
    },
}

from waflib.Build import BuildContext, CleanContext

for ARCH, OS_NAMES in PLATFORMS.items():
    for OS_NAME in OS_NAMES:
        for BUILD_TYPE in BUILD_TYPES:
            for CONTEXT in ( BuildContext, CleanContext ):
                name = CONTEXT.__name__.replace( 'Context', '' ).lower()
                class tmp( CONTEXT ):
                    osName = OS_NAME
                    arch = ARCH
                    variant = ARCH + '_' + OS_NAME + '_' + BUILD_TYPE
                    cmd = variant + '_' + name

def options( _ctx ):
    _ctx.add_option(
        '--' + common.DPDIR,
        type = 'string',
        default = os.path.join(
            '..',
            'dp',
        ),
    )

    for ARCH, OS_NAMES in PLATFORMS.items():
        for OS_NAME in OS_NAMES:
            if OS_NAME == common.LINUX:
                _ctx.load( 'compiler_cxx' )
            elif OS_NAME == common.WINDOWS:
                _ctx.load( 'msvc' )

def configure( _ctx ):
    for ARCH, OS_NAMES in PLATFORMS.items():
        for OS_NAME in OS_NAMES:
            for BUILD_TYPE in BUILD_TYPES:
                _ctx.setenv( ARCH + '_' + OS_NAME + '_' + BUILD_TYPE )
                _ctx.env.CXXFLAGS = CXXFLAGS[ OS_NAME ][ common.COMMON ] + CXXFLAGS[ OS_NAME ][ BUILD_TYPE ]
                _ctx.env.LINKFLAGS = LINKFLAGS[ OS_NAME ][ common.COMMON ] + LINKFLAGS[ OS_NAME ][ BUILD_TYPE ]

                _ctx.env[ common.DPDIR ] = os.path.abspath( _ctx.options.dpdir )

                if OS_NAME == common.LINUX:
                    if ARCH == common.X86:
                        _ctx.env.CXX = 'i686-pc-linux-gnu-g++'
                    elif ARCH == common.AMD64:
                        _ctx.env.CXX = 'x86_64-pc-linux-gnu-g++'

                    _ctx.env.LINKFLAGS.append(
                        ','.join(
                            [
                                '-Wl',
                                '-rpath-link',
                                ':'.join(
                                    common.generateLibpath( _ctx )
                                ),
                            ]
                        )
                    )

                    _ctx.load( 'compiler_cxx' )
                elif OS_NAME == common.WINDOWS:
                    if ARCH == common.X86:
                        _ctx.env.MSVC_TARGETS = 'x86'
                    elif ARCH == common.AMD64:
#                        _ctx.env.MSVC_TARGETS = 'x64'   # vs2010用
                        _ctx.env.MSVC_TARGETS = 'x86_amd64' # vs2012用

                    _ctx.load( 'msvc' )


from wscripts import demos

def build( _ctx ):
    if not _ctx.variant:
        _ctx.fatal( '[ ARCH ]_[ OS_NAME ]_[ debug | release ]_[ build | clean ]' )

    demos.build( _ctx )
