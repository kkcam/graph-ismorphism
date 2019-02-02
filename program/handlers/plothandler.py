#! /usr/bin/python2.7

"""
Logic that handles the plotting of data into graphs
"""

import matplotlib
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from numpy.random import uniform, seed
from matplotlib.mlab import griddata
import matplotlib.pyplot as plt
import operator


class PlotHandler(object):
    def plot_plot_2d(self, title, x, y, **kwargs):
        """
        Plot a generic 2d line graph
        :param title: 
        :param x: 
        :param y: 
        :param kwargs: 
        :return: 
        """
        plt.title(title)
        plt.xlabel(kwargs.get("x_label", "Set X"))
        plt.ylabel(kwargs.get("y_label", "Set Y"))
        plt.scatter(x, y)
        plt.plot(x, y)
        if kwargs.get("timed_out_x", False):
            plt.scatter(kwargs["timed_out_x"], kwargs["timed_out_y"], c='red')
        plt.ylim(ymin=0)
        plt.grid()

        # Save / Show
        if kwargs.get("save", False):
            plt.savefig("./../assets/graphs_run/" + title)
        if kwargs.get("hide", False):
            pass
        else:
            plt.show()
        plt.clf()
        plt.close()

    def plot_scatter_2d(self, title, x, y, z, **kwargs):
        """
        Plot a generic 2d scatter graph
        :param title: 
        :param x: 
        :param y: 
        :param z: 
        :param kwargs: 
        :return: 
        """
        plt.title(title)
        plt.xlabel(kwargs.get("x_label", "Set X"))
        plt.ylabel(kwargs.get("y_label", "Set Y"))
        plt.scatter(x, y)
        plt.ylim(ymin=0)
        plt.grid()

        # Save / Show
        if kwargs.get("save", False):
            plt.savefig("./../assets/graphs_run/" + title)
        if kwargs.get("hide", False):
            pass
        else:
            plt.show()
        plt.clf()
        plt.close()

    def plot_heatmap_2d(self, title, x, y, z, **kwargs):
        """
        Plot a generic 2d heatmap
        :param title: 
        :param x: 
        :param y: 
        :param z: 
        :param kwargs: 
        :return: 
        """
        plt.title(title)
        plt.xlabel(kwargs.get("x_label", "X"))
        plt.ylabel(kwargs.get("y_label", "Y"))
        plt.scatter(x, y, c=z, s=500)
        clb = plt.colorbar()
        clb.ax.set_title('Time(sec)')
        plt.grid()

        # Save / Show
        if kwargs.get("save", False):
            plt.savefig("./../assets/graphs_run/" + title)
        if kwargs.get("hide", False):
            pass
        else:
            plt.show()
        plt.clf()
        plt.close()

    def plot_graph_3d(self, title, x, y, z, **kwargs):
        """
        Plot a generic 3d scatter graph
        :param title: 
        :param x: 
        :param y: 
        :param z: 
        :param kwargs: 
        :return: 
        """
        fig = plt.figure()
        plt.title(title)
        ax = fig.add_subplot(111, projection='3d')
        ax.set_title(title)
        ax.scatter(x, y, z, c='r', marker='o')
        ax.set_xlabel('N Values')
        ax.set_ylabel('M Values')
        ax.set_zlabel('Time (sec)')
        plt.grid()

        # Save / Show
        if kwargs.get("save", False):
            plt.savefig("./../assets/graphs_run/" + title)
        if kwargs.get("hide", False):
            pass
        else:
            plt.show()
        plt.clf()
        plt.close()

    def plot_graphs_results(self, results, **kwargs):
        """
        Handler for plotting results generated by Traces
        :param results: 
        :param kwargs: 
        :return: 
        """
        kwargs["x_label"] = "nodes"
        kwargs["y_label"] = "time(sec)"
        for graph in results:
            self.plot_graph(results, graph, **kwargs)

    def plot_graph(self, results, graph, **kwargs):
        """
        Plot a single graph 
        - Helper to plot gi results
        :param results: 
        :param graph: 
        :param kwargs: 
        :return: 
        """
        x_axis = []
        y_axis = []
        timed_out_x = []
        timed_out_y = []

        # Sort nodes
        for result in results[graph]:
            result["nodes"] = int(result["nodes"])
        results[graph].sort(key=operator.itemgetter("nodes"))
        for result in results[graph]:

            # Some code to deal with nodes with multiple entries
            if result['nodes'] in x_axis:
                pos = x_axis.index(result["nodes"])
                # y_axis[pos] = "%.2f" % (float(y_axis[pos]) + float(result["time"]) * 0.5) # avg
                v = y_axis[pos] if y_axis[pos] > result["time"] else result["time"]
                y_axis[pos] = v
                if result["nodes"] in timed_out_x:
                    pos = timed_out_x.index(result["nodes"])
                    timed_out_y[pos] = v
                continue

            # Deal with timeouts
            if result["time"] == -1 or result["d_time"] == -1:
                print "timed out"
                result["time"] = y_axis[-1]
                timed_out_x.append(result['nodes'])
                timed_out_y.append(result["time"])

            x_axis.append(result['nodes'])
            y_axis.append(result['time'])
        kwargs["timed_out_x"] = timed_out_x
        kwargs["timed_out_y"] = timed_out_y

        if kwargs.get("get", False):
            return x_axis, y_axis, kwargs
        else:
            self.plot_plot_2d(graph, x_axis, y_axis, **kwargs)

    def plot_graphs_comparison(self, results):
        """
        A 2d plot for comparing graphs run on Traces
        :param results: 
        :return: 
        """
        # Init
        x_axis = range(10, 100, 10) + range(100, 1000, 100) + range(1000, 10000, 1000)
        x_label = "nodes"
        y_label = "time(sec)"
        title = "Graph comparison"
        plots = []

        for graph in results:
            x, y, kwargs = self.plot_graph(results, graph, get=True)
            plot = {
                "x": x,
                "y": y
            }
            plots.append(plot)

        # Plotting
        plt.title(title)
        plt.xlabel(x_label)
        plt.ylabel(y_label)
        # plt.xlim([1, 4000])
        for plot in plots:
            plt.plot(plot["x"], plot["y"])
        plt.grid()
        plt.show()
        plt.clf()
        plt.close()

    def plot_construction_results(self, results):
        """
        A 3d plot for constructions run on Traces
        :param results: 
        :return: 
        """
        graph = "construction_custom"
        x = []
        y = []
        z = []
        for result in results[graph]:
            n = int(result['name'].split("_")[0])
            m = int(result['name'].split("_")[1])
            t = float(result["time"])
            x.append(n)
            y.append(m)
            z.append(t)
        self.plot_graph_3d("Should work", x, y, z)

    def plot_sat_results(self, data):
        """
        Handler for plotting results from generating systems
        :param data: 
        :return: 
        """
        x = []
        y = []
        z = []
        for r in data:
            if r[0] == 'key':
                continue
            elif r[4] > 0:
                x.append(r[1])
                y.append(r[2])
                z.append(r[4])

        title = 'Sat run 0-n-10000_0-m-10000_step-100'

        self.plot_heatmap_2d(title,
                             x,
                             y,
                             z,
                             x_label="N Values",
                             y_label="M Values")
        self.plot_graph_3d(title, x, y, z)

    def plot_k_consistency_check(self, data):
        """
        Handler for plotting results for testing systems in Sat Solver
        :param data: 
        :return: 
        """
        # Time off - on (positive values are good)
        x = []
        y = []
        z = []
        for r in data:
            x.append(int(r[1]))
            y.append(int(r[2]))
            z.append(float(r[3]))  # off
            # z.append(float(r[4])) # on
            # z.append(float(r[5])) # off - on

        title = 'Sat run 0-n-10000_0-m-10000_step-100'
        self.plot_heatmap_2d(title,
                             x,
                             y,
                             z,
                             x_label="N Values",
                             y_label="M Values")
        self.plot_graph_3d(title, x, y, z)
