from sat import Sat
from profiling import Profiling
from bruteforce import BruteForce
from out import Out
import signal
import timeit
import pprint
import random
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


class Controller(object):
    def __init__(self):
        sat = Sat()
        bfr = BruteForce()
        # results = sat.generate_systems_stats()
        # data = self.generate_n_m()
        data = self.generate_n_m_incrementally()
        # data = self.load_n_m()
        self.plot_graph_3d(data)

    def generate_n_m(self):
        output = Out()
        output.update_file('results.txt', data)
        data = output.read_from_file('results.txt')
        return data

    def generate_n_m_incrementally(self):
        pro = Profiling()
        output = Out()
        pro.generate_n_m_incrementally('results.txt')
        data = output.read_from_file('results.txt')
        return data

    def load_n_m(self):
        out = Out()
        return out.read_from_file('results.txt')

    def plot_graph_2d(self, data):
        x = []
        y = []
        for r in data:
            if r[0] == 'key':
                continue
            elif r[4] > 0:
                x.append(r[1])
                y.append(r[2])

        plt.xlabel('N values')
        plt.ylabel('M values')
        plt.scatter(x, y)
        plt.show()

    def plot_graph_3d(self, data):
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

        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        ax.scatter(x, y, z, c='r', marker='o')
        ax.set_xlabel('N Values')
        ax.set_ylabel('M Values')
        ax.set_zlabel('Time')
        plt.show()

    def main(self):
        """Writing logic from Prof. Dawars handout
        """
        # Set of variables (including negations)
        X = []
        for i in range(-4, 5):
            X.append(i)

        # Set of clauses
        phi = []
        c1 = [1, 2, 3]
        c2 = [-3, 4, 5]
        c3 = [-2, 1, 3]
        phi = [c1, c2]

        # Satisfiable assignment
        # -4, -3, -2, -1, 0, 1, 2, 3, 4
        T = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        # System over the two element field
        S = []
        s1 = [1, 1, 1, 0]
        s2 = [0, 1, 1, 1]
        s3 = [0, 1, 1, 1]

        # Pebbles
        k = 6
        pebbles = [1, 2, 3, 4, 5, 6]
        while True:
            pi = random.choice(pebbles)
            x = random.choice(random.choice(phi))
            duplicator = random.choice([0, 1])

controller = Controller()
