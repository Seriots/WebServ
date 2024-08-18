#******************************************************************************#
#                                                                              #
#                                                         :::      ::::::::    #
#    test_configure_location.py                         :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gtoubol <marvin@42.fr>                     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/29 10:59:35 by gtoubol           #+#    #+#              #
#    Updated: 2022/11/29 10:59:35 by gtoubol          ###   ########.fr        #
#                                                                              #
#******************************************************************************#

import pytest
import os
import uuid
import subprocess

BASE_PATH = "/srcs/config/"


class TestConfigure:
    path = f"{os.getcwd()}{BASE_PATH}"
    exec_test = f"{path}test_configure.test"

    def run_error(
        self,
        filename: str,
        expected_log: str,
        expected_returncode: int
        ) -> None:
        """Run the test executable and compare the error output and returncode

        """
        with subprocess.Popen([self.exec_test, filename], stderr=subprocess.PIPE) as proc:
            errorlog = proc.stderr.read().decode()
            returncode = proc.wait()
        assert errorlog == expected_log
        assert returncode == expected_returncode

    def test_inputfile_location_empty(self, tmp_path):
        tmp = tmp_path / str(uuid.uuid4())
        tmp.write_text("""
server:
  location
  location:
  location: test test
  location: test
  location: /dsff-___-asdf-/

server:
  location: /coucou
  location: /test
  location: /coucou/ici/
  location: /coucou/
  location: /test/
#  location: /../test/../../test/coucou./..
""")

        self.run_error(
        tmp,
        """Bad config: line 3: location: missing delimiter
Bad config: line 4: location: invalid value
Bad config: line 5: location: invalid value
Bad config: line 6: location: location needs to start with a `/`
Bad config: line 13: location: `/coucou/` is already defined
Bad config: line 14: location: `/test/` is already defined
""",
        1)

    def test_inputfile_location_properties(self, tmp_path):
        tmp = tmp_path / str(uuid.uuid4())
        tmp.write_text("""server:
  root: /test
  index: ls
  location: /root
    index: ls- --
    root: /ls/ls
""")

        self.run_error(
            tmp,
            """""",
            0)

    def test_inputfile_location_permissions(self, tmp_path):
        tmp = tmp_path / str(uuid.uuid4())
        tmp.write_text("""
server:
  permissions: 7
  location: /coucou
    permissions:
  location: /test
    permissions: 2a
""")

        self.run_error(
            tmp,
            """Bad config: line 7: permissions: invalid value
""",
            1)

    def test_inputfile_location_max_body_size(self, tmp_path):
        tmp = tmp_path / str(uuid.uuid4())
        tmp.write_text("""
server:
  permissions: 7
  location: /coucou
    permissions:
    max_body_size:
  max_body_size: -25

""")

        self.run_error(
            tmp,
            """Bad config: line 6: max_body_size: invalid value
Bad config: line 7: max_body_size: invalid value
""",
            1)


    def test_inputfile_location_autoindex(self, tmp_path):
        tmp = tmp_path / str(uuid.uuid4())
        tmp.write_text("""
server:
  permissions: 7
  location: /coucou
    permissions:
    autoindex: on
  autoindex: off
server:
  autoindex: test

""")

        self.run_error(
            tmp,
            """Bad config: line 9: autoindex: invalid value
""",
            1)
