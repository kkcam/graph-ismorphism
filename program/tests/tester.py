from program.handlers.filehandler import FileHandler
from program.sat import Sat

fh = FileHandler()
sat = Sat()

system = fh.read_from_file("./../../assets/systems/2600_6900")
print sat.is_system_uniquely_satisfiable(system, 2600)