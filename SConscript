from building import *
import os

cwd = GetCurrentDir()
src = Glob('src/*.c')
CPPPATH = [cwd]

LOCAL_CFLAGS = ''

group = DefineGroup('LCD', src, depend = ['PKG_USING_NES_SIMULATOR'], CPPPATH = CPPPATH, LOCAL_CFLAGS = LOCAL_CFLAGS)

Return('group')
