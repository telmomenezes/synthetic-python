import random

from synthetic.consts import DEFAULT_MAX_DIST


class RandomWalker:
    def __init__(self, net, node, directed, max_length):
        self.net = net
        self.orig = node
        self.directed = directed
        self.max_length = max_length

        self.targ = None
        self.length = -1
        self.forward = False

        self.steps = [-1] * (max_length + 1)

        self.restart()

    def restart(self):
        self.targ = self.orig
        self.length = 0
        self.forward = random.choice([True, False])

    def step(self):
        if self.directed:
            if self.forward:
                neighbors = self.net.out_neighbors(self.targ)
            else:
                neighbors = self.net.in_neighbors(self.targ)
        else:
            neighbors = self.net.neighbors(self.targ)

        if len(neighbors) > 0:
            next_node = random.choice(neighbors)
        else:
            next_node = None

        if next_node is None:
            self.restart()
        elif self.orig == next_node:
            self.restart()
        elif next_node in self.steps:
            self.restart()
        else:
            self.steps[self.length] = next_node
            self.length += 1

            self.targ = next_node

            if self.length > self.max_length:
                self.restart()


class RandomWalkers:
    def __init__(self, net, directed):
        self.net = net
        self.directed = directed

        self.max_length = DEFAULT_MAX_DIST
        self.large_value = self.max_length + 1
        self.steps = 1

        self.nodes = net.graph.vcount()

        self.dmatrix = None
        self.walkers = None

        self.init()
    
    def init(self):
        # clear matrices
        self.dmatrix = [self.large_value] * (self.nodes * self.nodes)
        for i in range(self.nodes):
            self.dmatrix[(i * self.nodes) + i] = 0
        
        # init walkers
        self.walkers = [RandomWalker(self.net, node, self.directed, self.max_length) for node in range(self.nodes)]

    def distance(self, x, y):
        if x == y:
            return 0
        else:
            return self.dmatrix[int((y * self.nodes) + x)]

    def set_distance(self, x, y, d):
        if self.distance(x, y) <= d:
            return

        self.dmatrix[(y * self.nodes) + x] = d
        
        if not self.directed:
            self.dmatrix[(x * self.nodes) + y] = d
    
    def step(self):
        for walker in self.walkers:
            if self.net.degree(walker.orig) > 0:
                for _ in range(self.steps):
                    walker.step()
                    if walker.forward:
                        self.set_distance(walker.orig, walker.targ, walker.length)
                    else:
                        self.set_distance(walker.targ, walker.orig, walker.length)
    
    def all_steps(self):
        for _ in range(self.net.graph.ecount()):
            self.step()

    def recompute(self):
        self.init()
        self.all_steps()
