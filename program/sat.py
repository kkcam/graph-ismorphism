from pycryptosat import Solver
import numpy
import copy
import random
import pprint


class Sat(object):
    def test_case(self):
        # Init
        solver = Solver()
        variables = 5
        system = [
            [1, 2, 3],
            [2, 3, 4],
            [3, 4, 5],
            [1, 2, 5],
            [2, 3, 5],
        ]  # Assume clause, False <=> x, y, z = 0

        # Build
        for clause in system:
            solver.add_xor_clause(clause, False)

        # Exec
        sat, sol = solver.solve()
        uniqueSat = self.is_system_uniquely_satisfiable(system)

    def generate_rand_system(self, n, m):
        """Generates a random homogenous system
        """

        pool = range(1, n + 1)
        system = []
        tries = 3
        i = 0

        while i < m:
            clause = random.sample(pool, 3)
            clause.sort()

            if tries == 0:
                return False
            elif clause in system:
                tries -= 1
            else:
                system.append(clause)
                i += 1

        return system

    def generate_rand_unique_system(self, n, m):
        """Generates a random homogenous system that is uniquely satisfiable
        """

        tries = 3

        while True:
            system = self.generate_rand_system(n, m)

            if tries == 0:
                return False
            elif system:
                if self.is_system_uniquely_satisfiable(system):
                    return system
                else:
                    tries -= 1
            else:
                tries -= 1

    def generate_systems_fix_n(self):
        """Generate a system forcing n to stay static and allow m to vary
        Give up after t tries
        """

        sat = Sat()
        n = 50
        max_m = 1000
        tries = 10

        for m in range(555, max_m):
            eq = sat.generate_rand_unique_system(n, m)
            if eq:
                tries = 10
                print 'Found: ' + `m`
                # print eq
            else:
                print 'Couldnt find for ' + `m`
                tries -= 1
                print tries
                if tries == 0:
                    print 'Sequence of misses'
                    return

    def generate_systems_fix_n_force(self):
        """Generate a system forcing n to stay static and allowing m to vary
        Don't give up
        """
        sat = Sat()
        n = 50
        m = 555
        max_m = 1000
        misses = 0
        while m < max_m:
            m += 1
            eq = sat.generate_rand_unique_system(n, m)
            if eq:
                misses = 0
                print "Found: ", m
                # print eq
            else:
                misses += 1
                m -= 1
                print 'Couldnt find for ' + `m` + ' Misses ' + `misses`

    def generate_pool(self, matrix):
        """Gets a pool of variables used in a matrix
        """

        pool = []

        for clause in matrix:
            for i in clause:
                if i not in pool:
                    pool.append(i)

        return pool

    def is_system_uniquely_satisfiable(self, system):
        """Tests if a matrix is uniquely satisfiable
        """

        pool = self.generate_pool(system)

        for i in pool:
            solver = Solver()

            for clause in system:
                solver.add_xor_clause(clause, False)

            solver.add_xor_clause([i], True)
            sat, sol = solver.solve()

            if sat:
                return False

        return True
