#! /usr/bin/python2.7

"""
Logic for generating and timing graphs using Traces package
"""

import datetime
import re
import signal
from handlers.exceptionhandler import signal_handler, TimeoutError
from handlers.filehandler import FileHandler
from handlers.processhandler import ProcessHandler
import networkx as nx
import numpy as np
import math


class Gi(object):
    def run_all_graphs(self, **kwargs):
        """
        Run all graphs on dreadnaut
        :param kwargs: 
        :return: 
        """
        graphs = self.load_graphs()
        results = self.run_graphs(graphs, **kwargs)
        return results

    def run_graphs(self, graphs, **kwargs):
        """
        Run a set of graphs
        :param graphs: 
        :param kwargs: 
        :return: 
        """
        results = {}

        for graph in graphs:
            results[graph] = self.run_graph(graphs[graph], graph, **kwargs)

        return results

    def run_graph(self, graphs, graph, **kwargs):
        """
        Run instances in a graph
        :param graph: 
        :param kwargs: 
        :return: 
        """
        fh = FileHandler()
        ph = ProcessHandler()
        run = ph.run_command("ls -v ./../assets/graphs_run/")
        save = kwargs.get("save", False)
        outstanding = kwargs.get("outstanding", False)

        # Load previous results
        if outstanding and graph + ".txt" in run:
            graph_results = fh.read_from_file("./../assets/graphs_run/{0}.txt".format(graph))
        else:
            graph_results = []

        # Gather results
        for graph_instance in graphs:
            print "{0} {1}".format(graph_instance, datetime.datetime.now())

            # Skip existing graph
            if outstanding and graph + ".txt" in run:
                if any(d['name'] == graph_instance for d in graph_results):
                    continue


            graph_results.append(self.run_graph_instance(graph, graph_instance, **kwargs))

            # Save
            if save:
                print "Saving..."
                fh.write_to_file("./../assets/graphs_run/" + graph + ".txt", graph_results)

        return graph_results

    def run_graph_instance(self, graph, graph_instance, **kwargs):
        """
        Run a specific instance on dreadnaut
        dreadnaut At -a V=0 -m <"[path]" x q
        :param graph: 
        :param graph_instance: 
        :param kwargs: 
        :return: 
        """
        # Init
        ph = ProcessHandler()
        path = "./../assets/graphs/" + graph + "/" + graph_instance
        nodes = re.search("(n=?)=\d+", ' '.join(ph.run_command("head '" + path + "'"))).group(0)[2:]
        process = ph.open_process("dreadnaut")

        # Set timeout
        signal.signal(signal.SIGALRM, signal_handler)
        signal.alarm(kwargs.get("timeout", kwargs.get("timeout", 0)))

        # Gather results
        try:
            time, (stdout, stderr) = ph.run_function_timed(process.communicate,
                                                           ('At -a V=0 -m <"' + path + '" x q',),
                                                           return_args=True)
            split = re.search('(time=?) = \d+.\d+\d+', stdout)
            if split:
                d_time = split.group(0)[7:]
            else:
                time = -1
                d_time = -1

        except TimeoutError:
            print "Timed out: Took too long to validate"
            time = -1
            d_time = -1
            process.kill()
            process.terminate()
        finally:
            signal.alarm(0)

        return {
            "name": graph_instance,
            "nodes": nodes,
            "time": time,
            "d_time": d_time
        }

    def load_graphs(self):
        """
        Load graph instances from package
        :return: 
        """
        ph = ProcessHandler()
        graphs = {}
        for graph in ph.run_command('ls -v ./../assets/graphs/'):
            graphs[graph] = []
            for graph_instance in ph.run_command('ls -v ./../assets/graphs/' + graph):
                graphs[graph].append(graph_instance)
        return graphs

    def load_results(self):
        """
        Load results from tests
        :return: 
        """
        ph = ProcessHandler()
        fh = FileHandler()
        results = {}
        run = ph.run_command("ls -v ./../assets/graphs_run/")
        for graph_run in run:
            results[graph_run[:-4]] = fh.read_from_file("./../assets/graphs_run/" + graph_run)
        return results

    def generate_random_graphs(self):
        """
        Extend random graphs by generating larger versions provided in the Traces package.
        Random graphs are defined with edge probability 1/2, 1/10 and sqrt(n)
        :return: 
        """
        ph = ProcessHandler()
        instances = [5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 100, 200, 300,
                     400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000,
                     10000, 20000, 30000]

        probabilities = ["1/2", "1/10", "sqrt"]
        names = ["ran2_custom", "ran10_custom", "ransq_custom"]

        for p, n in zip(probabilities, names):
            print p
            for i in instances:
                dest = "./../assets/graphs_custom/{0}/{1}".format(n, i)
                print dest
                p = "1/" + str(int(math.ceil(math.sqrt(float(i)))))
                print "./../assets/nauty26r7/genrang -P{0} {1} 1 {2}.g6".format(p, i, dest)
                ph.run_command("./../assets/nauty26r7/genrang -P{0} {1} 1 {2}.g6".format(p, i, dest))
                ph.run_command("./../assets/nauty26r7/showg -d {0}.g6 {1}.dre".format(dest, dest))
                # ph.run_command("rm ./../assets/{0}.g6".format(dest))

    def convert_graph_to_traces(self, n, m, G, type, dir):
        """
        Convert a given networkx graph into dreadnaut format
        :param n: 
        :param m: 
        :param G: 
        :return: 
        """
        if type == "B":
            nodes = (2 * n) + (4 * m)
            variables = (2 * n)
        else:
            nodes = n + m
            variables = n

        # Init
        fh = FileHandler()
        path = dir + "{0}_{1}_{2}.dre".format(n, m, type)
        path_temp = "./../assets/temp/temp.adjlist"

        # Convert to Adjlist and store temporarily
        nx.write_adjlist(G, path_temp)

        # Read data and convert
        data = fh.read_from_file_simple(path_temp)
        output = ["n={0} $=0 g".format(nodes)]
        for i in range(3, variables + 3):
            datum = data[i].split()
            datum[0] = "{0}:".format(datum[0])
            output.append(" ".join(datum))
        output[-1] = "{}.".format(output[-1])
        output.append("$$")

        # Save as .dre
        fh.write_to_file_simple(path, output)

        # Convert to dot if necessary
        # ./nauty26r7/dretodot construction/3.dre construction/3.dot

        return path

    def graph_has_automorphisms(self, path):
        """
        Check if a given graph has any automorphisms 
        :param path: 
        :return: True of if has, False otherwise
        """
        ph = ProcessHandler()
        process = ph.open_process("dreadnaut")
        time, (stdout, stderr) = ph.run_function_timed(process.communicate,
                                                       ('At -a V=0 -m <"' + path + '" x q',),
                                                       return_args=True)

        return False if stdout.split(";")[2].split()[0] == '0' else True

    def is_graph_slower(self, path_a, path_b):
        """
        Determine if a graph at a saved location runs slower than another
        :param path_a: 
        :param path_b: 
        :return: 
        """
        ph = ProcessHandler()
        process = ph.open_process("dreadnaut")
        time_a, (stdout, stderr) = ph.run_function_timed(process.communicate,
                                                         ('At -a V=0 -m <"' + path_a + '" x q',),
                                                         return_args=True)
        time_b, (stdout, stderr) = ph.run_function_timed(process.communicate,
                                                         ('At -a V=0 -m <"' + path_b + '" x q',),
                                                         return_args=True)

        return time_a > time_b
