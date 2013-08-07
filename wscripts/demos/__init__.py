# -*- coding: utf-8 -*-

from . import args
from . import gamepad
from . import display

def build( _ctx ):
    args.build( _ctx )
    gamepad.build( _ctx )
    display.build( _ctx )
