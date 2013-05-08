#   Picman-Python - allows the writing of Picman plugins in Python.
#   Copyright (C) 1997  James Henstridge <james@daa.com.au>
#
#    This program is free software: you can redistribute it and/or modify
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

#   plugin.py -- helper for writing picman plugins
#     Copyright (C) 1997, James Henstridge.
#
# This is a small wrapper that makes plugins look like an object class that
# you can derive to create your plugin.  With this wrapper, you are pretty
# much responsible for doing everything (checking run_mode, gui, etc).  If
# you want to write a quick plugin, you probably want the picmanfu module.
#
# A plugin using this module would look something like this:
#
#   import picman, picmanplugin
#
#   pdb = picman.pdb
#
#   class myplugin(picmanplugin.plugin):
#       def query(self):
#           picman.install_procedure("plug_in_mine", ...)
#
#       def plug_in_mine(self, par1, par2, par3,...):
#           do_something()
#
#   if __name__ == '__main__':
#       myplugin().start()

import picman

class plugin:
    def start(self):
        picman.main(self.init, self.quit, self.query, self._run)

    def init(self):
        pass

    def quit(self):
        pass

    def query(self):
        pass

    def _run(self, name, params):
        if hasattr(self, name):
            return apply(getattr(self, name), params)
        else:
            raise AttributeError, name

if __name__ == '__main__':
    plugin().start()
