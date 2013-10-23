# -*- coding: utf-8 -*-

from . import args

from . import gamepad

from . import display

from . import window
from . import keyboard
from . import mouse

from . import opengl_simple

from . import audiooutput_simple

from . import readfile_simple
from . import readfilesize_simple
from . import writefile_simple
from . import writereadfile_simple
from . import appendfile_simple
from . import appendreadfile_simple

def build( _ctx ):
    args.build( _ctx )

    gamepad.build( _ctx )

    display.build( _ctx )

    window.build( _ctx )
    keyboard.build( _ctx )
    mouse.build( _ctx )

    opengl_simple.build( _ctx )

    audiooutput_simple.build( _ctx )

    readfile_simple.build( _ctx )
    readfilesize_simple.build( _ctx )
    writefile_simple.build( _ctx )
    writereadfile_simple.build( _ctx )
    appendfile_simple.build( _ctx )
    appendreadfile_simple.build( _ctx )
