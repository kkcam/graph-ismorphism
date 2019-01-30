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