# -*- coding: utf-8 -*-

from wscripts import common

import builder

def build( _ctx ):
    sources = {
        'main',
    }

    libraries = {
        common.generateLibraryName( 'common' ),
        common.generateLibraryName( 'audio' ),
    }

    builder.build(
        _ctx,
        'audiooutput_simple',
        sources,
        libraries = libraries,
    )
