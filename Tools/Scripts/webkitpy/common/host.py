# Copyright (c) 2010 Google Inc. All rights reserved.
# Copyright (c) 2009 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


from webkitpy.common.checkout import Checkout
from webkitpy.common.checkout.scm import default_scm
from webkitpy.common.config.ports import WebKitPort
from webkitpy.common.memoized import memoized
from webkitpy.common.net import bugzilla, buildbot, statusserver, web
from webkitpy.common.net.buildbot.chromiumbuildbot import ChromiumBuildBot
from webkitpy.common.net.irc import ircproxy
from webkitpy.common.system import executive, filesystem, platforminfo, user, workspace
from webkitpy.common.watchlist.watchlistloader import WatchListLoader
from webkitpy.layout_tests.port.factory import PortFactory


class Host(object):
    def __init__(self):
        # These basic environment abstractions should be a separate lower-level object.
        # Alternatively the rest of the objects in Host should just move up to a higher abstraction.
        self.executive = executive.Executive()
        self.filesystem = filesystem.FileSystem()
        self.user = user.User()
        self.platform = platforminfo.PlatformInfo()
        self.workspace = workspace.Workspace(self.filesystem, self.executive)
        self.web = web.Web()

        # FIXME: Checkout should own the scm object.
        self._scm = None
        self._checkout = None

        # Everything below this line is WebKit-specific and belongs on a higher-level object.
        self.bugs = bugzilla.Bugzilla()
        self.buildbot = buildbot.BuildBot()

        self._irc = None
        self._port = None

        self.status_server = statusserver.StatusServer()

        # FIXME: Unfortunately Port objects are currently the central-dispatch objects of the NRWT world.
        # In order to instantiate a port correctly, we have to pass it at least an executive, user, scm, and filesystem
        # so for now we just pass along the whole Host object.
        self.port_factory = PortFactory(self)

    def _initialize_scm(self, patch_directories=None):
        self._scm = default_scm(patch_directories)
        self._checkout = Checkout(self.scm())

    def scm(self):
        return self._scm

    def checkout(self):
        return self._checkout

    def port(self):
        return self._port

    @memoized
    def chromium_buildbot(self):
        return ChromiumBuildBot()

    @memoized
    def watch_list(self):
        return WatchListLoader(self.filesystem).load()

    def ensure_irc_connected(self, irc_delegate):
        if not self._irc:
            self._irc = ircproxy.IRCProxy(irc_delegate)

    def irc(self):
        # We don't automatically construct IRCProxy here because constructing
        # IRCProxy actually connects to IRC.  We want clients to explicitly
        # connect to IRC.
        return self._irc

    def command_completed(self):
        if self._irc:
            self._irc.disconnect()
