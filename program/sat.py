#! /usr/bin/python2.7

"""
Logic for producing uniquely satisfiable instances using Cryptominisat package
"""

import random
from pycryptosat import Solver
from handlers.filehandler import FileHandler
from handlers.processhandler import ProcessHandler
import timeit
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
import itertools


class Sat(object):
    def generate_systems(self, **kwargs):
        """
        Generate instances by searching through combinations of n and m
        Save these results as files
        
        :param kwargs: 
        :return: Results of time taken to search
        """
        # Init
        fh = FileHandler()
        ph = ProcessHandler()
        all_results = [['key', 'n', 'm', 'tries', 'vTime']]
        all_systems = []

        # Looping params
        step = kwargs.get("step", 1)
        max_tries = kwargs.get("max_tries", 30)
        min_m = kwargs.get("min_m", 4)
        n = kwargs.get("n", 4)
        max_n = kwargs.get("max_n", 100)
        max_m = kwargs.get("max_m", 100)
        # Complex looping params
        efficient_search = kwargs.get("limited_search", False)
        limit = kwargs.get("limit", False)
        upper_bound = kwargs.get("upper_bound", 4)
        lower_bound = kwargs.get("lower_bound", 1)
        # Additional params
        save_results_dir = "./../assets/sat_run/{0}-n-{1}_{2}-m-{3}_step-{4}".format(n, max_n, min_m, max_m, step)
        save_results = kwargs.get("save_results", False)
        save_systems = kwargs.get("save_systems", False)
        gi = kwargs.get("gi", False)
        update_strongly_k = kwargs.get("update_strongly_k", False)

        # Prep results folder
        if save_results or save_systems or gi:
            save_results_dir = fh.makedir(save_results_dir)
            save_results_location = save_results_dir + "/results"
            save_systems_location = save_results_dir + "/systems/"
            save_constructions_location = save_results_dir + "/constructions/"
            fh.makedir(save_systems_location)
            fh.makedir(save_constructions_location)

        # Loop n value
        while n <= max_n:
            # Init loop
            tries = 0
            found = 0
            smallest_m_found = False
            n_results = []
            n_systems = []

            if min_m < n:
                # If m is smaller than n, then bring m up to speed
                m = lower_bound * n
            else:
                m = min_m

            # Loop m value
            while m <= max_m:
                # Handle Iterators
                if max_tries == tries:
                    # Failed to find and tried too many times
                    print "Skipping: {0} {1}".format(n, m)
                    tries = 0
                    all_results.append([key, n, m, tries, -1, -1])
                    n_results.append([key, n, m, tries, -1, -1])
                    m += step
                    continue
                elif m > (upper_bound * n) or (found and found == limit):
                    # Do not search for m > 4n or continue to next m if adequate systems are found
                    break

                # Generate random system and record time taken to find
                key = `n` + ':' + `m`
                validation_start = timeit.default_timer()
                generate_time, system = ph.run_function_timed(self.generate_rand_system, (n, m),
                                                              return_args=True)

                # Validate system
                if self.is_system_uniquely_satisfiable(system, n) \
                        and ((gi and self.is_system_eligble(n, m, system, gi, save_results_dir)) or not gi) \
                        and ((update_strongly_k and self.is_system_slower(n, m, system)) or not update_strongly_k):

                    # Found unique system
                    print "Found: {0} {1}".format(n, m)
                    # Record times
                    validation_time = timeit.default_timer() - validation_start
                    all_results.append([key, n, m, tries, validation_time, generate_time])
                    n_results.append([key, n, m, tries, validation_time, generate_time])
                    all_systems.append([key, n, m, system])
                    n_systems.append([key, n, m, system])

                    # Update iterators
                    tries = 0
                    found += 1
                    if efficient_search and not smallest_m_found:
                        # Update the lower bound
                        min_m = m - step
                        smallest_m_found = True

                    if update_strongly_k:
                        self.update_strongly_k(n, m, system)
                else:
                    # Failed to find, try again
                    # print 'Couldnt find for {0} {1} Misses {2}'.format(n, m, tries)
                    tries += 1
                    m -= step

                # Increment m
                m += step

            # Save search information
            if save_results:
                self.save_results(n_results, save_results_location)
            if save_systems:
                self.save_results_systems(n_systems, save_systems_location)

            # Increment n
            n += step

        return all_results, all_systems

    def generate_rand_system(self, n, m):
        """
        Generates a random homogenous system
        Try n times to pull a unique random set of three variables from the pool.
        - I.e. don't pick the same clause twice.
        :param n: 
        :param m: 
        :return: A 2d array of systems
        """
        # Init
        pool = range(1, n + 1)
        system = []
        tries = 3
        i = 0

        # Build an array of unique arrays as a system
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

    def generate_systems_fix_n(self):
        """
        Generate a system forcing n to stay static and allow m to vary
        Give up after n tries
        :return: Null
        """
        sat = Sat()
        n = 50
        max_m = 1000
        tries = 10

        for m in range(555, max_m):
            system = sat.generate_rand_system(n, m)
            if self.is_system_uniquely_satisfiable(system, n):
                tries = 10
                print 'Found: ' + `m`
            else:
                print 'Couldnt find for ' + `m`
                tries -= 1
                print tries
                if tries == 0:
                    print 'Sequence of misses'
                    return

    def generate_systems_fix_n_force(self):
        """
        Generate a system forcing n to stay static and allowing m to vary
        Don't give up, that is, keep trying until a system is found
        :return: Null
        """
        sat = Sat()
        n = 50
        m = 555
        max_m = 1000
        misses = 0
        while m < max_m:
            m += 1
            system = sat.generate_rand_system(n, m)
            if self.is_system_uniquely_satisfiable(system, n):
                misses = 0
                print "Found: ", m
                # print eq
            else:
                misses += 1
                m -= 1
                print 'Couldnt find for ' + `m` + ' Misses ' + `misses`

    def is_system_uniquely_satisfiable(self, system, n):
        """
        Tests unique satisfiable by banning all zero solution
        :param system: 
        :param n: 
        :return: 
        """
        if not system:
            return False

        # Prep solver
        solver = Solver()
        for clause in system:
            solver.add_xor_clause(clause, False)

        # Ban all zero
        solver.add_clause(range(1, n + 1))

        sat, sol = solver.solve()

        # print "Found system is {0}".format(sat)

        return not sat

    def is_system_eligble(self, n, m, system, gi, location):
        """
        Check if graph meets the construction criteria
        - k consistent 
        - No automorphisms
        :param n: 
        :param m: 
        :param gi: 
        :param system: 
        :return: 
        """
        # Init
        ph = ProcessHandler()
        fh = FileHandler()
        graph_path = "{0}/constructions/".format(location)
        system_path = "{0}/constructions/{1}_{2}".format(location, n, m)
        construct_a_location = "{0}{1}_{2}_A.dre".format(graph_path, n, m)

        # Save system temporarily
        self.save_system(n, m, system, graph_path)
        G = self.convert_system_to_graph(n, m, system)
        gi.convert_graph_to_traces(n, m, G, "A", graph_path)  # First construction

        # Check for k-local consistency
        if not self.is_k_consistent(n, m, system):
            # print "Not K consistent"
            fh.delete_file(system_path)
            return False
        elif not gi.graph_has_automorphisms(construct_a_location):
            # print "No Automorphisms. Construct."
            G = self.convert_system_to_construction(n, m, system)
            gi.convert_graph_to_traces(n, m, G, "B", graph_path)  # Second construction
            fh.delete_file(system_path)
            return True
        else:
            # print "Automorphisms. Remove."
            fh.delete_file(construct_a_location)  # Remove unwanted graph
            fh.delete_file(system_path)  # Remove unwanted system
            return False

    def is_system_slower(self, n, m, system):
        """
        Determine if a given system is slower than the current slowest system stored in "systems_strongly_k"
        :param n: 
        :param m: 
        :param system: 
        :return: 
        """
        fh = FileHandler()
        path = "./../assets/systems_strongly_k/{0}_{1}".format(n, m)
        temp_path_a = "./../assets/temp/temp_a"
        temp_path_b = "./../assets/temp/temp_b"
        system_old = fh.read_from_file(path)

        if not system_old:
            return True

        fh.write_to_file_simple(temp_path_a, self.prepare_cryptominisat_system(n, m, system))
        fh.write_to_file_simple(temp_path_b, self.prepare_cryptominisat_system(n, m, system_old))
        diff_a = self.get_gauss_off_time(temp_path_a) - self.get_gauss_on_time(temp_path_a)
        diff_b = self.get_gauss_off_time(temp_path_b) - self.get_gauss_on_time(temp_path_b)

        if diff_a > diff_b:
            print "Slower {0}".format(diff_a - diff_b)
        pass

        return diff_a > diff_b

    def update_strongly_k(self, n, m, system):
        """
        Update the folder of stored systems which are a catalogue of slow systems
        :param n: 
        :param m: 
        :param system: 
        :return: 
        """
        ph = ProcessHandler()
        fh = FileHandler()
        path = "./../assets/systems_strongly_k/{0}_{1}".format(n, m)
        fh.delete_file(path)
        self.save_system(n, m, system, "./../assets/systems_strongly_k/")

    def find_equations(self, n, m):
        """
        Find unique systems by searching combinations, rather than picking at random.
        That is, find systems of equations "systematically"
        :param n: 
        :param m: 
        :return: 
        """
        clauses = self.find_clauses(n)
        systems = self.find_systems(clauses, [], n, m, 0, find_one=False)
        return systems

    def find_clauses(self, n):
        """
        Helper that finds all unique combinations of clauses
        :param n: 
        :return: A 2d array of all combinations of 
        """
        pool = range(1, n + 1)
        clauses = list(itertools.combinations(pool, 3))
        return clauses

    def find_systems(self, clauses, system, n, m, depth, **kwargs):
        """
        Find all systems using recursive method
        :param clauses: 
        :param system: 
        :param n: 
        :param m: 
        :param depth: 
        :return: A system of equations
        """

        # Base Case
        # If length of system = m, then we have long enough system
        if len(system) == m:
            for i in range(0, len(system) - 1):
                if system[i] > system[i + 1]:
                    return False
            # print system
            sat = Sat()
            if sat.is_system_uniquely_satisfiable(system, n):
                return True
            else:
                return False

        # Recursive Case
        # Else, system is not long enough, we need to append to system
        else:
            systems = []

            # For each clause not in the system, add to system
            for clause in clauses:

                # Remove this clause from the pool
                tail = list(clauses)
                tail.remove(clause)

                # Add this clause to the current system and validate
                system_temp = list(system)
                system_temp.append(clause)
                unique_system = self.find_systems(tail, system_temp, n, m, depth + 1)

                # Check if it is a uniquely satisfiable instance or is it a return call
                if isinstance(unique_system, bool) and unique_system:
                    systems.append(system_temp)
                    if kwargs.get("find_one", False):
                        return systems
                elif unique_system:
                    systems = systems + unique_system

            return systems

    def run_solver(self, **kwargs):
        """
        Run Traces through systems and record times
        Looking for systems that are faster with gauss off => K-local consistent
        :param kwargs: 
        :return: Null
        """
        fh = FileHandler()
        ph = ProcessHandler()
        results = []
        skip = kwargs.get("outstanding", False)
        completed = []
        if fh.read_from_file("./../assets/systems_run/run"):
            for result in fh.read_from_file("./../assets/systems_run/run"):
                completed.append(result[0])

        for filename in ph.run_command('ls -v ./../assets/systems/'):
            # Init
            path = './../assets/systems/' + filename
            system = fh.read_from_file(path)
            split = filename.split("_")
            n = split[0]
            m = split[1]

            # Skip completed systems
            key = "{0}:{1}".format(n, m)
            if skip and key in completed:
                continue
            print key

            # Create cryptominisat system
            time_off, time_on = self.get_gauss_times(n, m, system)

            # Save
            results.append([key, n, m, time_off, time_on, time_off - time_on])
            fh.update_file("./../assets/systems_run/run", results)

    def prepare_cryptominisat_system(self, n, m, system):
        """
        Helper to translate a python system to a cryptominisat system
        Save this translation into a file for processing
        :param n: 
        :param m: 
        :param system: 
        :return: 
        """
        # init
        input = [
            'p cnf {0} {1}'.format(n, m)
        ]

        # Grab clauses
        for clause in system:
            input.append("x{0} {1} -{2} 0".format(int(clause[0]), int(clause[1]), int(clause[2])))

        # Ensures uniquely satisfiable
        input.append(" ".join([str(i) for i in range(1, int(n) + 1)]) + " 0")

        # Return
        return input

    def find_pool(self, clauses):
        """
        Helper to find variables used
        Redundant
        :param clauses: 
        :return: A list of integers used in clauses
        """
        variables = []
        for clause in clauses:
            for variable in clause:
                if variable not in variables:
                    variables.append(variable)
        return variables

    def save_systems(self, systems, location):
        """
        Save a set of systems of equations to a file
        :param systems: 
        :return: 
        """
        for system in systems:
            # n, m, system
            self.save_system(system[1], system[2], system[3], location)

    def save_system(self, n, m, system, location):
        """
        Save a system of equations to file
        :param n: 
        :param m: 
        :param system: 
        :param location: 
        :return: 
        """
        fh = FileHandler()
        path = location + "{0}_{1}".format(n, m)
        fh.write_to_file(path, system)

    def save_results(self, results, location):
        """
        Save a set of results
        :param location: 
        :param results: 
        :return: 
        """
        fh = FileHandler()
        ph = ProcessHandler()
        print "Saving results..."
        save_results_time = ph.run_function_timed(fh.update_file, (location, results))
        print "Time taken: ", save_results_time

    def save_results_systems(self, systems, location):
        fh = FileHandler()
        ph = ProcessHandler()
        print "Saving systems..."
        save_systems_time = ph.run_function_timed(self.save_systems, (systems, location))
        print "Time taken: ", save_systems_time

    def load_results(self):
        """
        Load Sat Solver execution results
        :return: 
        """
        fh = FileHandler()
        results = fh.read_from_file("./../assets/systems_run/run")
        return results

    def convert_system_to_graph(self, n, m, system, **kwargs):
        """
        Convert a system of equations to a Traces graph
        First construction used in checking for Automorphisms
        :param system: 
        :return: 
        """

        # Empty matrix of correct size
        width = n + m
        A = np.zeros((width, width))

        # Insert edges
        c = 0
        for clause in system:
            for i in range(0, 3):
                A[clause[i] - 1][n + c] = 1
            # Increment
            c = c + 1

        # Make Symmetric
        A = np.maximum(A, A.transpose())

        # Prepare Positioning
        L = range(0, n)
        R = range(n, n + m)

        # Prepare Labels
        labels = range(1, n + 1) + ["C" + str(i) for i in range(1, m + 1)]
        labels_dict = {}
        for i in range(0, n + m):
            labels_dict[i] = labels[i]

        # Construct Graph
        G = nx.from_numpy_matrix(A)
        if kwargs.get("plot", False):
            pos = nx.spring_layout(G)
            pos = dict()
            pos.update((n, (i, 1)) for i, n in enumerate(R))  # put nodes from X at x=1
            pos.update((n, (i, 2)) for i, n in enumerate(L))  # put nodes from Y at x=2
            nx.draw(G, pos)
            nx.draw_networkx_labels(G, pos, labels_dict)
            plt.draw()
            plt.show()

        return G

    def convert_system_to_construction(self, n, m, system, **kwargs):
        """
        Convert a system of equations into a Traces graph
        Second construction after checking for automorphisms
        :param n: 
        :param m: 
        :param system: 
        :return: 
        """
        width = (2 * n) + (4 * m)
        A = np.zeros((width, width))

        # Insert edges
        c = 0
        for clause in system:
            c_pos = (2 * n) + c
            # 0 0 0
            A[(clause[0] * 2) - 2][c_pos] = 1
            A[(clause[1] * 2) - 2][c_pos] = 1
            A[(clause[2] * 2) - 2][c_pos] = 1
            # 0 1 1
            A[(clause[0] * 2) - 2][c_pos + 1] = 1
            A[(clause[1] * 2) - 1][c_pos + 1] = 1
            A[(clause[2] * 2) - 1][c_pos + 1] = 1
            # 1 0 1
            A[(clause[0] * 2) - 1][c_pos + 2] = 1
            A[(clause[1] * 2) - 2][c_pos + 2] = 1
            A[(clause[2] * 2) - 1][c_pos + 2] = 1
            # 1 1 0
            A[(clause[0] * 2) - 1][c_pos + 3] = 1
            A[(clause[1] * 2) - 1][c_pos + 3] = 1
            A[(clause[2] * 2) - 2][c_pos + 3] = 1

            # Increment
            c = c + 4

        # Make Symmetric
        A = np.maximum(A, A.transpose())

        # Prepare Positioning
        L = range(0, 2 * n)
        R = range(2 * n, width)

        # Prepare Labels
        labels_dict = {}
        labels = []
        for i in range(1, n + 1):
            labels.append("{0}F".format(i))
            labels.append("{0}T".format(i))
        for i in range(1, m + 1):
            labels.append("C{0}_1".format(i))
            labels.append("C{0}_2".format(i))
            labels.append("C{0}_3".format(i))
            labels.append("C{0}_4".format(i))
        for i in range(0, width):
            labels_dict[i] = labels[i]

        # Construct Graph
        G = nx.from_numpy_matrix(A)
        if kwargs.get("plot", False):
            pos = nx.spring_layout(G)
            pos = dict()
            pos.update((n, (i, 1)) for i, n in enumerate(R))  # put nodes from X at x=1
            pos.update((n, (i, 2)) for i, n in enumerate(L))  # put nodes from Y at x=2
            nx.draw(G, pos)
            nx.draw_networkx_labels(G, pos, labels_dict)
            plt.draw()
            plt.show()

        return G

    def is_k_consistent(self, n, m, system):
        """
        Looking for systems that are faster with gauss off => K-local consistent
              
        on vs off
        faster on versus off
        
        Looking for systems that are FASTER (take less time) with GAUSS ON than GAUSS OFF
        
        :param n: 
        :param m: 
        :param system: 
        :return: 
        """
        ph = ProcessHandler()
        fh = FileHandler()

        # Get times
        time_off, time_on = self.get_gauss_times(n, m, system)

        # If Gauss On - Gauss Off > Threshold (sec)
        # threshold = time_b - time_a > float(1)
        # I.e. time_off - time_on > 0
        threshold = time_on < time_off  # No threshold determined

        return threshold

    def get_gauss_times(self, n, m, system):
        """
        Retrieve the validation times (unique satisfiability) on a given system executed on a Sat solver
        :param n: 
        :param m: 
        :param system: 
        :return: 
        """
        path = "./../assets/temp/temp_gauss_check"
        fh = FileHandler()

        # Create cryptominisat system
        input = self.prepare_cryptominisat_system(n, m, system)
        fh.write_to_file_simple(path, input)

        # run gauss off
        time_off = self.get_gauss_off_time(path)

        # run gauss on
        time_on = self.get_gauss_on_time(path)

        return time_off, time_on

    def get_gauss_off_time(self, path):
        """
        Retrieve time taken to execute with gauss off
        = Helper
        :param path: 
        :return: 
        """
        ph = ProcessHandler()
        cmd = "./../assets/sat/cryptominisat/build/cryptominisat5 --verb=0 {0}".format(path)
        time, out = ph.run_function_timed(ph.run_command, (cmd,), return_args=True)
        return time

    def get_gauss_on_time(self, path):
        """
        Retrieve time taken to execute with gauss on
        - Helper
        :param path: 
        :return: 
        """
        ph = ProcessHandler()
        cmd = "./../assets/sat/cryptominisat/build_gauss/cryptominisat5 --verb=0 {0}".format(path)
        time, out = ph.run_function_timed(ph.run_command, (cmd,), return_args=True)
        return time
