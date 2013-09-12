# -*- coding: utf-8 -*-

from wscripts import common

import builder

def build( _ctx ):
    sources = {
        'main',
    }

    libraries = {
        common.generateLibraryName( 'common' ),
        common.generateLibraryName( 'window' ),
        common.generateLibraryName( 'opengl' ),
    }

    builder.build(
        _ctx,
        'opengl_simple',
        sources,
        libraries = libraries,
    )
