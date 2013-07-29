# -*- coding: utf-8 -*-

from wscripts import common

import builder

def build( _ctx ):
    sources = {
        'main',
    }

    libraries = {
        common.generateLibraryName( 'common' ),
    }

    builder.build(
        _ctx,
        'gamepad',
        sources,
        libraries = libraries,
    )
