# -*- coding: utf-8 -*-

from wscripts import common

import builder

def build( _ctx ):
    sources = {
        'main',
        'wav',
    }

    libraries = {
        common.generateLibraryName( 'common' ),
        common.generateLibraryName( 'audio' ),
        common.generateLibraryName( 'file' ),
    }

    builder.build(
        _ctx,
        'audiooutput_simple',
        sources,
        libraries = libraries,
    )
