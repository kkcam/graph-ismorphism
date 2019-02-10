#! /usr/bin/python2.7

"""
Main logic
"""

from sat import Sat
from gi import Gi
from main_logic import Main
from handlers.filehandler import FileHandler
from handlers.plothandler import PlotHandler
from handlers.processhandler import ProcessHandler

# Tests
class Tester(object):
    def test_1(self):
        """
        Search 1: 1 - 100
        :return: 
        """
        main = Main()
        main.generate_systems(n=4,
                            min_m=4,
                            max_n=100,
                            max_m=100,
                            step=1,
                            save_results=False,
                            save_systems=True,
                            limit=False,
                            max_tries=30)


    def test_2(self):
        """
        Search 2: 100 - 1000
        + 10
        :return: 
        """
        main = Main()
        main.generate_systems(n=100,
                            min_m=100,
                            max_n=1000,
                            max_m=1000,
                            step=10,
                            save_results=False,
                            save_systems=True,
                            limit=False,
                            max_tries=10)


    def test_3(self):
        """
        Search 3: 1000 - 10000
        + 100
        :return: 
        """
        main = Main()
        main.generate_systems(n=1000,
                            min_m=1000,
                            max_n=10000,
                            max_m=10000,
                            step=100,
                            save_results=True,
                            save_systems=True,
                            limit=10,
                            efficient_search=True,
                            max_tries=10)


    def test_4(self):
        """
        Search 4: 10000 - 50000
        + 1000
        :return: 
        """
        main = Main()
        main.generate_systems(n=10000,
                            min_m=10000,
                            max_n=50000,
                            max_m=50000,
                            step=1000,
                            save_results=False,
                            save_systems=True,
                            limit=5,
                            max_tries=5)


    def test_5(self):
        """
        Run sat solver through saved instances
        :return: 
        """
        main = Main()
        main.execute_sat_solver()
        main.plot_sat_solver_results()


    def test_6(self):
        """
        Convert saved instances to graphs and run
        :return: 
        """
        main = Main()
        main.convert_systems_to_constructions(validate=True, delete=False)
        main.execute_constructions()
        # main.plot_constructions_results()


    def test_7(self):
        """
        Run saved .dre graphs through Traces and time results
        :return: 
        """
        main = Main()
        main.execute_graphs(outstanding=True,
                            timeout=10800,
                            save=True)

        # main.plot_graphs_results(save=True)


    def test_8(self):
        """
        Use recursion to find instances for a given n and m rather than randomly searching
        :return: 
        """
        sat = Sat()
        x = sat.find_equations(5, 6)
        for i in x:
            print x.count(i), sat.is_system_uniquely_satisfiable(i, 5)
            for j in x:
                if i == j:
                    continue


    def test_9(self):
        """
        Construction search
        Generate systems that also fulfil construction criteria
        - k consistent
        - No automorphisms
        
        Slow
        Systems are saved to construction_search
        :return: 
        """
        main = Main()
        main.generate_systems(n=4,
                            min_m=4,
                            max_n=100,
                            max_m=100,
                            step=10,
                            save_results=True,
                            save_systems=True,
                            gi=Gi())


    def test_10(self):
        """
        Threshold search 1: 1000 - 10000
        :return: 
        """
        main = Main()
        main.generate_systems(n=1000,
                            min_m=1000,
                            max_n=10000,
                            max_m=10000,
                            step=100,
                            save_results=True,
                            save_systems=True,
                            limit=10)
        main.plot_generate_systems_results('./../assets/sat_run/0-n-10000_0-m-10000_step-100/results', aggregate=True)


    def test_11(self):
        """
        Construction search 2
        Generate systems that also fulfil construction criteria
        - k consistent
        - No automorphisms

        Slow
        Systems are saved to construction_search
        :return: 
        """
        main = Main()
        main.generate_systems(n=1000,
                            min_m=1000,
                            max_n=10000,
                            max_m=20000,
                            step=1000,
                            save_results=True,
                            save_systems=True,
                            upper_bound=2,
                            lower_bound=1,
                            max_tries=30,
                            gi=Gi())


    def test_12(self):
        """
        Update strongly k directory
        :return: 
        """
        main = Main()
        main.generate_systems(n=10,
                            min_m=10,
                            max_n=100,
                            max_m=200,
                            step=10,
                            save_results=True,
                            save_systems=True,
                            upper_bound=3,
                            lower_bound=1,
                            max_tries=30,
                            update_strongly_k=True,
                            gi=Gi())


    def test_13(self):
        """
        Find strongly k 
        4 - 10
        10 - 100
        100 - 1000
        1000 - 10000
        :return: 
        """
        main = Main()
        for i in range(0, 30):
            main.generate_systems(n=10,
                                min_m=10,
                                max_n=100,
                                max_m=300,
                                step=10,
                                save_results=True,
                                save_systems=True,
                                upper_bound=3,
                                lower_bound=1,
                                max_tries=30,
                                update_strongly_k=True,
                                gi=Gi())


    def test_14(self):
        """
        Recursive search
        Warning - may crash
        :return: 
        """
        sat = Sat()
        n = 10
        m = 10
        clauses = sat.find_clauses(n)
        systems = sat.find_systems(clauses, [], n, m, 0, find_one=True)
        return systems


    def test_15(self):
        """
        Demonstrating difficulty in finding n = m
        :return: 
        """
        main = Main()
        main.generate_systems(n=100,
                            min_m=100,
                            max_n=200,
                            max_m=200,
                            step=1,
                            save_results=True,
                            save_systems=True,
                            upper_bound=1,
                            lower_bound=1,
                            max_tries=10000,
                            update_strongly_k=True,
                            gi=Gi())


    def test_16(self):
        """
        Demonstrating difficulty in finding 2n = m
        :return: 
        """
        main = Main()
        main.generate_systems(n=10,
                            min_m=10,
                            max_n=100,
                            max_m=100,
                            step=10,
                            save_results=True,
                            save_systems=True,
                            upper_bound=1,
                            lower_bound=1,
                            max_tries=1000,
                            update_strongly_k=True,
                            gi=Gi())


    def test_17(self):
        fh = FileHandler()
        data = fh.read_from_file("../assets/results/sat/0-n-10000_0-m-10000_step-100/results")
        for d in data:
            print "{0}  {1} a".format(d[1], d[2])
        i = [10,20,30,40,50,60,70,80,90,100, 110, 120]
        gi = Gi()
        for j in i:
            a = gi.run_graph_instance("ga_vs_gb", "{0}_{1}_A.dre".format(j,j))
            b = gi.run_graph_instance("ga_vs_gb", "{0}_{1}_B.dre".format(j,j))
            print "{0}  {1} a".format(j,a["time"],1)
            print "{0}  {1} b".format(j,b["time"],1)


    def test_18(self): 
        main = Main()
        main.execute_graphs(outstanding=True,
                            timeout=10800,
                            save=True)

        main.plot_graphs_results(save=True)


    def test_19(self):
        main = Main()
        main.execute_graphs(timeout=5400, save=True, useAll=True)