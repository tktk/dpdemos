# -*- coding: utf-8 -*-

from wscripts import common

import builder

def build( _ctx ):
    sources = {
        'main',
    }

    libraries = {
        common.generateLibraryName( 'common' ),
        common.generateLibraryName( 'file' ),
    }

    builder.build(
        _ctx,
        'writereadfile_simple',
        sources,
        libraries = libraries,
    )
