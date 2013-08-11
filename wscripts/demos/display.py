# -*- coding: utf-8 -*-

from wscripts import common

import builder

def build( _ctx ):
    sources = {
        'main',
    }

    libraries = {
        common.generateLibraryName( 'common' ),
        common.generateLibraryName( 'display' ),
    }

    builder.build(
        _ctx,
        'display',
        sources,
        libraries = libraries,
    )
