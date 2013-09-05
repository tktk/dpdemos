# -*- coding: utf-8 -*-

from . import args

from . import gamepad

from . import display

from . import window

from . import opengl_simple

def build( _ctx ):
    args.build( _ctx )

    gamepad.build( _ctx )

    display.build( _ctx )

    window.build( _ctx )

    opengl_simple.build( _ctx )
