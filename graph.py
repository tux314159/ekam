#! /usr/bin/python

from collections import deque
from typing import Union
import copy

Node = Union[str, int]

class Graph:
    def __init__(self) -> None:
        self.adjlist: dict[Node, set[Node]] = {}
        self.indegs: dict[Node, int] = {}

    def addNode(self, n: Node) -> None:
        if n not in self.adjlist:
            self.adjlist[n] = set()
            self.indegs[n] = 0

    def addEdge(self, v1: Node, v2: Node) -> None:
        self.adjlist[v1].add(v2)
        self.indegs[v2] += 1

    def addEdges(self, vs: list[tuple[Node, Node]]) -> None:
        for vp in vs:
            self.addEdge(vp[0], vp[1])

    def removeEdge(self, v1: Node, v2: Node) -> None:
        self.adjlist[v1].remove(v2)
        self.indegs[v2] -= 1


def getOrder(graph: Graph, changed: list[Node]) -> list[Node]:
    partgraph: Graph = Graph()
    q: deque = deque(changed)

    # Push all updated nodes
    for v in changed:
        partgraph.addNode(v)

    # BFS to construct partial graph
    while len(q) != 0:
        c = q.pop()

        for adj in graph.adjlist[c]:
            q.append(adj)
            partgraph.addNode(adj)
            partgraph.addEdge(c, adj)

    # Topo sort
    del(q)
    q = deque(changed) # These all have in-degree 0
    ordered = copy.deepcopy(changed)
    while len(q) != 0:
        c = q.pop()
        for adj in partgraph.adjlist[c]:
            partgraph.indegs[adj] -= 1 # Yes, yes I know this is really bad
            if partgraph.indegs[adj] == 0:
                ordered.append(adj)
            q.append(adj)

    return ordered






if __name__ == "__main__":
    g = Graph()
    for i in range(17):
        g.addNode(i)
    g.addEdges([(1,0), (2,0), (3,0), (9,0), (4,1), (6,2), (7,3), (8,3),
                (5,4), (7,5), (7,6), (10,9), (11,10), (14,10), (12,11),
                (13,12), (15,14), (16,15)])
    print(getOrder(g, [13, 8, 7]))
