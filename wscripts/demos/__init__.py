# -*- coding: utf-8 -*-

from . import args

from . import gamepad

from . import display

from . import window
from . import keyboard
from . import mouse

from . import opengl_simple

from . import audiooutput_simple

def build( _ctx ):
    args.build( _ctx )

    gamepad.build( _ctx )

    display.build( _ctx )

    window.build( _ctx )
    keyboard.build( _ctx )
    mouse.build( _ctx )

    opengl_simple.build( _ctx )

    audiooutput_simple.build( _ctx )
