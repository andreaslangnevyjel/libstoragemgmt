# Copyright (C) 2015-2016 Red Hat, Inc.
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; If not, see <http://www.gnu.org/licenses/>.
#
# Author: Gris Ge <fge@redhat.com>

import hashlib
import os
from typing import List

import redis
from mwct.tools import process_tools

__all__ = [
    "cmd_exec",
    "ExecError",
]


class DelayedRedis(object):
    def __init__(self):
        self._instance = None
        self._instance_init = False

    def init(self):
        from mwct.control.service.instance import InstanceXML
        from mwct.host_monitor_server.client_enums import ServiceEnum
        self._instance = InstanceXML(quiet=True)
        self._cache_port = self._instance.get_port_dict(
            ServiceEnum.redis_server,
            command=True,
        )
        self._instance_init = True
        self._cache_addr = "127.0.0.1"

    def get_cache(self):
        if not self._instance_init:
            self.init()
        return redis.Redis(
            host=self._cache_addr,
            port=self._cache_port,
            db=0,
        )

    @staticmethod
    def get_md5(cmds: List) -> str:
        cur = hashlib.new("md5")
        for cmd in cmds:
            cur.update(cmd.encode("utf-8"))
        return "cmd_{}".format(cur.hexdigest())

    @staticmethod
    def _exec(cmds: List) -> process_tools.CallResult:
        result = process_tools.call_command(
            cmds,
            environment={
                "PATH": os.getenv("PATH"),
            },
            universal_newlines=True,
            log_stdout=False,
            dynamic_read=True,
            log_to_result=True,
            encoding="ascii",
        )
        if result.retcode not in [0, 46]:
            raise ExecError(
                " ".join(cmds),
                result.retcode,
                result.stdout,
                result.stderr,
            )
        return result

    def cmd_exec(self, cmds: List[str]) -> process_tools.CallResult:
        cur_md5 = self.get_md5(cmds)
        mc = self.get_cache()
        previous = mc.get(cur_md5)
        if previous is None:  # or True:
            cur_result = self._exec(cmds)
            # print(cur_result, cur_result.retcode)
            mc.set(
                cur_md5,
                cur_result.to_json(),
                30,
            )
            return cur_result
        else:
            return process_tools.CallResult.from_json(previous)


dmc_obj = DelayedRedis()


def cmd_exec(cmds) -> process_tools.CallResult:
    """
    Execute provided command and return the STDOUT as string.
    Raise ExecError if command return code is not zero
    """
    return dmc_obj.cmd_exec(cmds)


class ExecError(Exception):
    def __init__(self, cmd, errno, stdout, stderr, *args):
        Exception.__init__(self, *args)
        self.cmd = cmd
        self.errno = errno
        self.stdout = stdout
        self.stderr = stderr

    def __str__(self) -> str:
        return "cmd: '{}', errno: {:d}, stdout: '{}', stderr: '{}'".format(
            self.cmd,
            self.errno,
            self.stdout,
            self.stderr,
        )
