from sat import Sat
import signal
import timeit
import pprint


class BruteForce(object):
    def find_equations(self, n, m):
        clauses = self.find_clauses(n)
        systems = self.find_systems(clauses, n, m)
        return systems

    def find_clauses(n):
        pool = range(1, n + 1)
        clauses = []
        for x in pool:
            for y in pool:
                for z in pool:
                    if x == y or x == z or y == z:
                        continue

                    clause = [x, y, z]
                    clause.sort()

                    if clause not in clauses:
                        clauses.append(clause)
        return clauses

    def find_systems(self, clauses, n, m):
        return self.find_system(clauses, [], n, m, 0)

    def find_system(self, clauses, system, n, m, depth):
        systems = []
        if depth > m:
            # print 'M: '+`m`
            # print 'Depth: '+`depth`
            # print 'something wierd is happening'
            # print 'System: '+`system`
            # print 'clauses: '+`clauses`
            return False

        # If length of system = m , then we have long enough system
        if len(system) == m:
            for i in range(0, len(system) - 1):
                if system[i] > system[i + 1]:
                    return False
            # print system
            sat = Sat()
            if sat.is_system_uniquely_satisfiable(system):

                return system
            else:
                return False

        # Else system is not long enough, we need to append to system
        else:
            # For each clause not in the system, add to system
            for clause in clauses:
                tail = list(clauses)
                tail.remove(clause)
                if clause not in system:
                    systemTemp = list(system)
                    systemTemp.append(clause)
                    sys = self.find_system(tail, systemTemp, n, m, depth + 1)
                    if sys:
                        return sys
        return False
