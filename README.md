mbtools
=======

Description
-----------

Modbus tools based on libmodbus to log data fetched by a master/client
or received by a slave/server (writing of registers). mbcollect is able
to act as client or server (in TCP or RTU):
- in client/master mode, it reads addresses defined by the user at regular
  interval and output results to a local Unix socket
- in server/slave mode, it waits for requests from other devices and output
  register writes to a local Unix socket.

Requirements
------------

automake libtool pkg-config libglib2.0-dev libmodbus >= v3.1.1

Installation
------------

    $ ./autogen.sh
    $ ./configure
    $ make


Testing
-------

To run the full suite of tests, you need to connect two USB-serial adapters on
the same link. The tests assume there seen as ttyUSB0 and ttyUSB1.
You must be able to open port 1502 on your localhost for TCP unit tests.
Once you're ready, you can launch the 4 tests with:

    $ cd tests
    $ python ./testcase.py


To launch one test at once, you must specify the class of TestCase:

   $ python -m unittest testcase.ClientTestCase


Settings
--------

The program mbcollect accepts many options from command line, all options from
command line are also available in config .ini file. Some options related to
remote slave/server when in master/client mode are only available in .ini file.

To list all available options:

    $ ./mbcollect --help

All these options are also available in the *[settings}* section of config .ini
file:

    [settings]
    mode = client
    socketfile = /tmp/mbsocket
    interval = 1
    verbose = 1


To define, the servers/slaves to read you can add one or many sections like this
one in your config .ini file:

    [server "faraway"]
    # A server called 'faraway' with IP 192.18.0.5
    ip=192.168.0.5
    port=502
    # Read 4 integers at address 0
    # Read 1 float at address 10
    addresses=0;10;
    lengths=4;1;
    types=int;floatmsb;

If *mbcollect* runs in:

- *client* mode, the *[settings]* and *[server]* sections will be used
- *master* mode, the *[settings]* and *[slave]* sections will be used, this
  section allows to define slave ID instead of IP address and port number.
- *server* and *slave* modes, only the common *settings* section will be used

See *tests/* for a list of config file examples.


Stop and reload
---------------

The programs are designed to handle SIGTERM to stop them. *mbcollect* is also
able to reload its config file on SIGHUP signal.
