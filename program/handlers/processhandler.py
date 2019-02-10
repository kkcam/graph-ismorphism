#! /usr/bin/python2.7

"""
Logic that handles running system processes
"""

import os
import subprocess
import timeit


class ProcessHandler(object):
    def run_command(self, command):
        """
        Run a command and return a list
        Good for single lined commands
        :param command: 
        :return: 
        """
        p = os.popen(command, "r")
        out = []
        while 1:
            line = p.readline()
            if not line:
                break
            out.append(line.rstrip())
        return out

    def open_process(self, command):
        """
        Prepare a process
        Useful for multiple inputs
        :param command: 
        :return: Return a subprocess object
        """
        process = subprocess.Popen([command], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        return process

    def run_process(self, command, inputs):
        """
        Execute a process
        Useful for multiple inputs
        :param command: 
        :param inputs: 
        :return: 
        """
        process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        outputs = []
        for input in inputs:
            print inputs
            outputs.append(process.stdin.write(input))
        print process.stdout
        return outputs,

    def run_function_timed(self, f, args, **kwargs):
        """
        Time a process
        Useful in running Sat Solver and Traces
        :param f: 
        :param args: 
        :param kwargs: 
        :return: 
        """
        # print args
        start = timeit.default_timer()
        ret = f(*args)
        time = timeit.default_timer() - start
        if kwargs.get("return_args"):
            return time, ret
        return time
    
    def run_command_timed(self, command):
        start = timeit.default_timer()
        out = self.run_command(command)
        time = timeit.default_timer() - start
        return time, out
