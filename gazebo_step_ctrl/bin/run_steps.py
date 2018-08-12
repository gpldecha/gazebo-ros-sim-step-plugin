#!/usr/local/bin/python

from pyipc import Xsi
import time
import signal
import sys

def signal_handler(sig, frame):
        print('You pressed Ctrl+C!')
        sys.exit(0)


if __name__ == '__main__':
	signal.signal(signal.SIGINT, signal_handler)

	xsi = Xsi()
	print('Start')
	while True:
		xsi.ping()
		time.sleep(0.001)
