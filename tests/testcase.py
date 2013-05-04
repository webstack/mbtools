#!/usr/bin/env python

from subprocess import Popen, PIPE
import os
import re
import signal
import unittest


class MasterTestCase(unittest.TestCase):

    def setUp(self):
        if os.path.exists("/tmp/mbsocket"):
            os.unlink("/tmp/mbsocket")

        self.uts = Popen(["./unit-test-server", "rtu"], stdout=PIPE, stderr=PIPE)
        self.rec = Popen(["../src/mbrecorder"], stdout=PIPE)
        self.col = Popen(["../src/mbcollect", "-f", "master-test-case.ini"], stdout=PIPE)
        self.is_master = True

    def tearDown(self):
        self.col.send_signal(signal.SIGTERM)
        self.rec.send_signal(signal.SIGTERM)
        self.uts.send_signal(signal.SIGTERM)
        self.col.wait()
        self.rec.wait()
        self.uts.wait()

    def test_output(self):
        # MBCOLLECT
        # First line contains host name
        line = self.col.stdout.readline()
        if self.is_master:
            m = re.search("Slave name (\w+), ID (\d+)", line)
            self.assertEqual(m.group(2), "1")
        else:
            m = re.search("Server name (\w+), IP ([\.\d]+):(\d+)", line)
            self.assertEqual(m.group(2), "127.0.0.1")
            self.assertEqual(m.group(3), "1502")

        server_name = m.group(1)
        self.assertEqual(server_name, "hello")

        # Check parsing of values to read
        c = re.compile("Address (\d+) => (\d+) values \((\w+)\)")

        expected_values = [
            {'a': 0, 'l': 2, 'type': 'floatmsb', 'values': ['12345.599609']},
            {'a': 2, 'l': 2, 'type': 'floatlsb', 'values': ['789.000000']},
            {'a': 0, 'l': 4, 'type': 'floatmsb', 'values': [
                '12345.599609', '0.000000']},
            {'a': 4, 'l': 1, 'type': 'int', 'values': ['1']},
            {'a': 4, 'l': 3, 'type': 'int', 'values': ['1', '2', '3']},
        ]
        for d in expected_values:
            m = c.match(self.col.stdout.readline())
            self.assertEqual(m.group(1), str(d['a']))
            self.assertEqual(m.group(2), str(d['l']))
            self.assertEqual(m.group(3), d['type'])

        line = self.col.stdout.readline()
        if self.is_master:
            self.assertEqual(line, "Opening /dev/ttyUSB0 at 115200 bauds (N, 8, 1)\n")
        else:
            self.assertEqual(line, "Connecting to 127.0.0.1:1502\n")

        # MBRECORDER
        # Ignore remaining output for collector and check recorder
        c = re.compile("mb_%s_(\d+) ([\d\.]+)([\||\n]?)" % server_name)

        for d in expected_values:
            line = self.rec.stdout.readline()

            if d['type'].find('int') == 0:
                l = d['l']
                step = 1
            else:
                l = d['l'] / 2
                step = 2

            start = 0
            for i in range(l):
                m = c.search(line, start)
                self.assertIsNotNone(m, "Item: %d" % i)
                # Address
                self.assertEqual(m.group(1), str(d['a'] + i * step))
                # Value read
                self.assertEqual(m.group(2), str(d['values'][i]))
                # Separator
                if i != l - 1:
                    self.assertEqual(m.group(3), '|')
                else:
                    self.assertEqual(m.group(3), '\n')

                start = m.end()


class ClientTestCase(MasterTestCase):

    def setUp(self):
        if os.path.exists("/tmp/mbsocket"):
            os.unlink("/tmp/mbsocket")

        self.uts = Popen(["./unit-test-server", "tcp"], stdout=PIPE, stderr=PIPE)
        self.rec = Popen(["../src/mbrecorder"], stdout=PIPE)
        self.col = Popen(["../src/mbcollect", "-f", "client-test-case.ini"], stdout=PIPE)
        self.is_master = False


class SlaveTestCase(unittest.TestCase):

    def setUp(self):
        if os.path.exists("/tmp/mbsocket"):
            os.unlink("/tmp/mbsocket")

        self.rec = Popen(["../src/mbrecorder"], stdout=PIPE)
        self.col = Popen(["../src/mbcollect", "-f", "slave-test-case.ini"], stdout=PIPE)
        self.uts = Popen(["./unit-test-client", "rtu"], stdout=PIPE, stderr=PIPE)

    def tearDown(self):
        self.col.send_signal(signal.SIGTERM)
        self.rec.send_signal(signal.SIGTERM)
        self.col.wait()
        self.rec.wait()
        # Client stops himself

    def test_recorder(self):
        # Client write only one register
        self.assertEqual(self.rec.stdout.readline(), "mb_0 1234\n")
        # Another query with two registers
        self.assertEqual(self.rec.stdout.readline(), "mb_1 5678|mb_2 9012\n")


class ServerTestCase(SlaveTestCase):
    """Inhrerits from SlaveTestCase test_recorder and tearDown"""

    def setUp(self):
        if os.path.exists("/tmp/mbsocket"):
            os.unlink("/tmp/mbsocket")

        self.rec = Popen(["../src/mbrecorder"], stdout=PIPE)
        self.col = Popen(["../src/mbcollect", "-f", "server-test-case.ini"], stdout=PIPE)
        self.uts = Popen(["./unit-test-client", "tcp"], stdout=PIPE, stderr=PIPE)


if __name__ == '__main__':
    unittest.main()
