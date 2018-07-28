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