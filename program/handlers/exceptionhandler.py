#! /usr/bin/python2.7

"""
Exception handlers
"""

class TimeoutError(Exception):
    pass

def signal_handler(signum, frame):
    raise TimeoutError()