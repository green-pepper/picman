#   Picman-Python - allows the writing of Picman plugins in Python.
#   Copyright (C) 1997  James Henstridge <james@daa.com.au>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

# picmanshelf.py -- a simple module to help picman modules written in Python
#                 store persistent data.
#
# Copyright (C) 1997, James Henstridge
#
# The picman module provides a basic method for storing information that persists
# for a whole picman session, but only allows for the storage of strings.  This
# is because other Python types usually have pointers to other Python objects,
# making it dificult to work out what to save.  This module gives an interface
# to the picman module's primitive interface, which resembles the shelve module.

# use cPickle and cStringIO if available

try:
    import cPickle as pickle
except ImportError:
    import pickle

try:
    import cStringIO as StringIO
except ImportError:
    import StringIO

import picman

import copy_reg

def _image_id(obj):
    return picman._id2image, (obj.ID,)

def _drawable_id(obj):
    return picman._id2drawable, (obj.ID,)

def _display_id(obj):
    return picman._id2display, (obj.ID,)

def _vectors_id(obj):
    return picman._id2vectors, (int(obj.ID),)

copy_reg.pickle(picman.Image,   _image_id,    picman._id2image)
copy_reg.pickle(picman.Layer,   _drawable_id, picman._id2drawable)
copy_reg.pickle(picman.Channel, _drawable_id, picman._id2drawable)
copy_reg.pickle(picman.Display, _display_id,  picman._id2display)
copy_reg.pickle(picman.Vectors, _vectors_id,  picman._id2vectors)

del copy_reg, _image_id, _drawable_id, _display_id, _vectors_id

class Picmanshelf:
    def has_key(self, key):
        try:
            s = picman.get_data(key)
            return 1
        except picman.error:
            return 0

    def __getitem__(self, key):
        try:
            s = picman.get_data(key)
        except picman.error:
            raise KeyError, key

        f = StringIO.StringIO(s)
        return pickle.Unpickler(f).load()

    def __setitem__(self, key, value):
        f = StringIO.StringIO()
        p = pickle.Pickler(f)
        p.dump(value)
        picman.set_data(key, f.getvalue())

    def __delitem__(self, key):
        picman.set_data(key, '')

shelf = Picmanshelf()
del Picmanshelf
