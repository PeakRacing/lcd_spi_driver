from building import *
import os

cwd = GetCurrentDir()
src = Glob('*.c')
CPPPATH = [cwd]

LOCAL_CFLAGS = ''

group = DefineGroup('LCD', src, depend = ['PKG_USING_LCD_SPI_DRIVER'], CPPPATH = CPPPATH, LOCAL_CFLAGS = LOCAL_CFLAGS)

Return('group')
