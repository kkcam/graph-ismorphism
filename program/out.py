from sys import argv
import pickle
import os.path
import operator


class Out(object):
    def write_to_file(self, filename, data):
        with open(filename, 'wb') as f:
            pickle.dump(data, f)

    def read_from_file(self, filename):
        with open(filename, 'rb') as f:
            data = pickle.load(f)
        return data

    def update_file(self, filename, data):

        if os.path.isfile(filename):
            old_data = self.read_from_file(filename)

            for datum1 in old_data:
                key1 = datum1[0]
                found = False
                for datum2 in data:
                    key2 = datum2[0]
                    if key1 == key2:
                        found = True
                        break
                if found == False:
                    data.append(datum1)

        data.sort(key=operator.itemgetter(1))
        self.write_to_file(filename, data)

        return data



