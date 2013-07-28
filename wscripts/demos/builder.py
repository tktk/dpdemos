# -*- coding: utf-8 -*-

from wscripts import common

import os.path

def build(
    _ctx,
    _sampleName,
    _sources,
    libraries = set(),
):
    common.buildDemo(
        _ctx,
        _sampleName,
        _generateIncludes(
            _ctx,
            _sampleName,
        ),
        common.generateLibpath( _ctx ),
        _generateSources(
            _sampleName,
            _sources,
        ),
        libraries,
    )

def _generateIncludes(
    _ctx,
    _sampleName,
):
    return _generateIncludesCommon( _ctx ) + [
        os.path.join(
            _generateDemosBaseDir( _sampleName ),
            common.INCLUDE_DIR,
        )
    ]

def _generateIncludesCommon( _ctx ):
    return [
        common.INCLUDE_DIR,
    ] + [
        os.path.join(
            _ctx.env[ i ],
            common.INCLUDE_DIR,
        )
        for i in [
            common.DPDIR,
        ]
    ]

def _generateSources(
    _sampleName,
    _sources,
):
    return {
        os.path.join(
            _generateDemosBaseDir( _sampleName ),
            common.SOURCE_DIR,
            s + '.cpp',
        )
        for s in _sources
    }

def _generateDemosBaseDir( _sampleName ):
    return os.path.join(
        common.DEMOS_DIR,
        _sampleName,
    )
