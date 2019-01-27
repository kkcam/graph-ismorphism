#! /usr/bin/python2.7

"""
Main logic
"""

from sat import Sat
from gi import Gi
from handlers.filehandler import FileHandler
from handlers.plothandler import PlotHandler
from handlers.processhandler import ProcessHandler


class Main(object):
    def generate_systems(self, **kwargs):
        """
        Generate instances
        :param kwargs: 
        :return: 
        """
        sat = Sat()
        results, systems = sat.generate_systems(**kwargs)
        return results, systems

    def plot_generate_systems_results(self, filename, **kwargs):
        """
        Plot time taken to generate instances
        :param filename: 
        :param kwargs: 
        :return: 
        """
        ph = PlotHandler()
        fh = FileHandler()
        data = fh.read_from_file(filename, **kwargs)
        ph.plot_sat_results(data)

    def execute_graphs(self, **kwargs):
        """
        Run graphs through Traces
        :param kwargs: 
        :return: 
        """
        gi = Gi()
        # kwargs["graphs"] = {
        #     "tnn": gi.load_graphs()["tnn"]
        # }
        if kwargs.get("graphs"):
            graphs = kwargs.get("graphs")
            kwargs.pop('graphs', None)
            gi.run_graphs(graphs, **kwargs)
        else:
            gi.run_all_graphs(**kwargs)

    def plot_graphs_results(self, **kwargs):
        """
        Plot time taken to run graphs through Traces
        :param kwargs: 
        :return: 
        """
        ph = PlotHandler()
        gi = Gi()
        results = gi.load_results()
        ph.plot_graphs_results(results, **kwargs)
        ph.plot_graphs_comparison(results)

    def execute_sat_solver(self):
        """
        Time execution time of running instances through Sat Solver
        :return: 
        """
        sat = Sat()
        sat.run_solver()

    def plot_sat_solver_results(self):
        """
        Plot execution time of running instances through Sat Solver
        :return: 
        """
        sat = Sat()
        ph = PlotHandler()
        results = sat.load_results()
        ph.plot_k_consistency_check(results)

    def convert_systems_to_constructions(self, **kwargs):
        """
        Convert found systems into graphs and run them through Traces
        :return: 
        """
        # Init
        gi = Gi()
        sat = Sat()
        ph = ProcessHandler()
        fh = FileHandler()
        paths = ph.run_command("ls -v ./../assets/systems_to_convert/")
        validate = kwargs.get("validate", False)
        delete = kwargs.get("delete", False)

        # Iterate systems
        for path in paths:
            print "Checking " + path

            # Paths
            graph_path = "./../assets/construction/" + path + "_A.dre"
            system_path = "./../assets/systems_to_convert/" + path

            # Extract n and m values
            n, m = path.split("_")
            n = int(n)
            m = int(m)

            # Load system
            system = fh.read_from_file(system_path)

            if validate:
                # Check for k-local consistency
                if not sat.is_k_consistent(n, m, system):
                    print "\t Not K consistent system. Removing and skipping."
                    if delete:
                        fh.delete_file(system_path)
                    continue
                else:
                    print "\t K consistent system. Constructing A."

                # Convert system into graphs and check for automorphisms
                G = sat.convert_system_to_graph(n, m, system)
                gi.convert_graph_to_traces(n, m, G, "A", "./../assets/construction/")  # First construction
                if not gi.graph_has_automorphisms(graph_path):
                    print "\t No Automorphisms. Constructing B."
                    G = sat.convert_system_to_construction(n, m, system)
                    gi.convert_graph_to_traces(n, m, G, "B", "./../assets/construction/")  # Second construction
                    if delete:
                        fh.delete_file(graph_path)
                else:
                    print "\t Automorphisms. Removing and skipping."
                    if delete:
                        fh.delete_file(graph_path)  # Remove unwanted graph
                        fh.delete_file(system_path)  # Remove unwanted system
            else:
                G = sat.convert_system_to_construction(n, m, system)
                gi.convert_graph_to_traces(n, m, G, "B", "./../assets/construction/")

    def execute_constructions(self):
        """
        Run new constructions through Traces
        :return: 
        """
        gi = Gi()
        fh = FileHandler()
        graphs = {
            "con_all": gi.load_graphs()["con_all"]
        }
        results = gi.run_graphs(graphs, save=True, timeout=7200)

        # Init
        con_4_10 = []
        con_10_100 = []
        con_100_1000 = []
        con_n = []
        con_2n = []
        con_3n = []
        con_sml = []
        found = []

        # Extract packages
        for result in results["con_all"]:
            n = int(result["name"].split("_")[0])
            m = int(result["name"].split("_")[1])
            if n in range(4, 10, 1):
                con_4_10.append(result)
            if n in range(10, 100, 10):
                con_10_100.append(result)
            if n in range(100, 1000, 100):
                con_100_1000.append(result)
            if n == m:
                con_n.append(result)
            if 2 * n == m:
                con_2n.append(result)
            if 3 * n == m:
                con_3n.append(result)

        # Extract smallest n : m ratio
        for i in results["con_all"]:
            n = int(i["name"].split("_")[0])
            m = int(i["name"].split("_")[1])

            if n in found:
                continue

            for j in results["con_all"]:
                n_1 = int(j["name"].split("_")[0])
                m_1 = int(j["name"].split("_")[1])
                if n == n_1 and m_1 <= m:
                    con_sml.append(j)
                    found.append(n)

        # Produce packages
        packages = {
            "con_4_10": con_4_10,
            "con_10_100": con_10_100,
            "con_100_1000": con_100_1000,
            "con_n": con_n,
            "con_2n": con_2n,
            "con_3n": con_3n,
            "con_sml": con_sml
        }

        # Save packages
        for package in packages:
            fh.write_to_file("./../assets/graphs_run/{0}.txt".format(package), packages[package])
            fh.makedir("./../assets/graphs/{0}".format(package))
            for instance in packages[package]:
                name = instance["name"]
                fh.copy_file("./../assets/graphs/con_all/{0}".format(name),
                             "./../assets/graphs/{0}/{1}".format(package, name))

        return results

    def plot_constructions_results(self):
        ph = PlotHandler()
        gi = Gi()
        r = gi.load_results()
        results = {
            "con_4_10": r["con_4_10"],
            "con_10_100": r["con_10_100"],
            "con_100_1000": r["con_100_1000"],
            "con_n": r["con_n"],
            "con_2n": r["con_2n"],
            "con_3n": r["con_3n"],
            "con_sml": r["con_sml"]
        }
        ph.plot_graphs_results(results)
        ph.plot_construction_results(results)


# Tests

def test_1():
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


def test_2():
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


def test_3():
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


def test_4():
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


def test_5():
    """
    Run sat solver through saved instances
    :return: 
    """
    main = Main()
    main.execute_sat_solver()
    main.plot_sat_solver_results()


def test_6():
    """
    Convert saved instances to graphs and run
    :return: 
    """
    main = Main()
    main.convert_systems_to_constructions(validate=True, delete=False)
    main.execute_constructions()
    # main.plot_constructions_results()


def test_7():
    """
    Run saved .dre graphs through Traces and time results
    :return: 
    """
    main = Main()
    main.execute_graphs(outstanding=True,
                        timeout=10800,
                        save=True)

    # main.plot_graphs_results(save=True)


def test_8():
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


def test_9():
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


def test_10():
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


def test_11():
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


def test_12():
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


def test_13():
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


def test_14():
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


def test_15():
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


def test_16():
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




if __name__ == "__main__":
    """
    Command line handling
    """
    # test_1() # Search
    # test_2() # Search
    # test_3() # Search
    # test_4() # Search
    # test_5() # Run Sat Solver
    # test_6()  # Convert systems to graphs and time them
    # test_7()  # Run Traces
    # test_8() # Recursive search
    # test_9() # Search
    # test_10() # Search
    # test_11() # Search
    # test_12() # Search
    # test_13() # Search
    # test_14() # Search
    # test_15() # Search
    # test_16() # Search

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