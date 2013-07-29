# -*- coding: utf-8 -*-

from . import args
from . import gamepad

def build( _ctx ):
    args.build( _ctx )
    gamepad.build( _ctx )
