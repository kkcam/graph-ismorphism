from sat import Sat
from out import Out
import signal
import timeit
import pprint


class Profiling(object):
    def generate_equations(self, maxN=20, maxM=20):
        results = [['variables', 'clauses', 'time to find', 'time to validate']]
        homSystems = []

        for n in range(3, maxN):
            for m in range(n + 1, maxM):
                vTime = -1

                # Find
                fTime, system = self.find_system(n, m)
                if system:
                    vTime = self.validate_system(system)
                    homSystems.append(system)
                    print 'found n:' + `n` + ' m:' + `m`
                # print system

                results.append([n, m, round(fTime, 3), round(vTime, 3)])

        # pp = pprint.PrettyPrinter(depth=6)
        # pp.pprint(results)`
        # pp.pprint(homSystems)
        return homSystems

    def generate_equation(self, n, m):
        results = [['variables', 'clauses', 'time to find', 'time to validate']]
        homSystems = []
        i = 0

        while True:
            fTime, system = self.find_system(n, m)
            if system:
                vTime = self.validate_system(system)
                homSystems.append(system)
                print 'found n:' + `n` + ' m:' + `m`
                # print system
                results.append([n, m, round(fTime, 3), round(vTime, 3)])
                pp = pprint.PrettyPrinter(depth=6)
                # pp.pprint(results)
                # pp.pprint(homSystems)
                return system
            else:
                i += 1
                print 'Tried ' + `i`

    def find_system(self, n, m):
        def signal_handler(signum, frame):
            raise Exception("Timed out!")

        sat = Sat()
        system = False
        signal.signal(signal.SIGALRM, signal_handler)
        signal.alarm(1)
        findTime = -1

        try:
            findStart = timeit.default_timer()
            system = sat.generate_rand_unique_system(n, m)
            findTime = timeit.default_timer() - findStart

        except Exception, msg:
            print "Timed out: Took too long to find a system"

        return (findTime, system)

    def validate_system(self, system):
        def signal_handler(signum, frame):
            raise Exception("Timed out!")

        signal.signal(signal.SIGALRM, signal_handler)
        signal.alarm(1)
        sat = Sat()
        validationTime = -1

        try:
            validationStart = timeit.default_timer()
            sat.is_system_uniquely_satisfiable(system)
            validationTime = timeit.default_timer() - validationStart

        except Exception, msg:
            print "Timed out: Took too long to validate"

        self.time = validationTime
        return self.time

    def generate_n_m(self):
        sat = Sat()
        results = [['key', 'n', 'm', 'misses', 'vTime']]
        max_misses = 10
        min_m = 4
        n = 4
        max_n = 100
        max_m = 100

        while n < max_n:
            m = min_m
            n += 1
            misses = 0

            while m < max_m:
                validation_start = timeit.default_timer()
                m += 1
                eq = sat.generate_rand_unique_system(n, m)
                key = `n` + ':' + `m`

                if eq:
                    print "Found: ", n, m
                    validation_time = timeit.default_timer() - validation_start
                    misses = 0
                    results.append([key, n,m,misses, validation_time])

                elif max_misses == misses:
                    # print "Skipping: ", n, m
                    misses = 0
                    results.append([key, n,m,misses, -1])
                    continue

                else:
                    # print 'Couldnt find for ' + `m` + ' Misses ' + `misses`
                    misses += 1
                    m -= 1

        return results

    def generate_n_m_incrementally(self, filename):
        output = Out()
        sat = Sat()
        results = [['key', 'n', 'm', 'misses', 'vTime']]
        max_misses = 10
        min_m = 100
        n = 99
        max_n = 100
        max_m = 200

        while n < max_n:
            m = min_m
            n += 1
            misses = 0

            while m < max_m:
                validation_start = timeit.default_timer()
                m += 1
                eq = sat.generate_rand_unique_system(n, m)
                key = `n` + ':' + `m`

                if eq:
                    print "Found: ", n, m
                    validation_time = timeit.default_timer() - validation_start
                    misses = 0
                    results.append([key, n,m,misses, validation_time])
                    output.update_file(filename, results)

                elif max_misses == misses:
                    # print "Skipping: ", n, m
                    misses = 0
                    results.append([key, n,m,misses, -1])
                    continue

                else:
                    # print 'Couldnt find for ' + `m` + ' Misses ' + `misses`
                    misses += 1
                    m -= 1

        return results

    def ppprint(self, data):
        pp = pprint.PrettyPrinter(depth=6)
        pp.pprint(data)
