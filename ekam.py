import hashlib
import os
from typing import Any

import graph

def getHash(src: str) -> Any:
    f = open(src, "rb")
    x = hashlib.sha256(f.read())
    f.close()
    return x


def getHashes(srcs: list[str]) -> dict[str, Any]:
    return {src: getHash(src) for src in srcs}

    
def updCheck(srcs: list[str], origHashes: dict[str, Any]):
    upd: list[str] = []
    for src in srcs:
        if getHash(src) != origHashes[src]:
            upd.append(src)
    return upd


class DepGraph:
    def __init__(self) -> None:
        self.g = graph.Graph()
        self.rules: dict[graph.Node, list[str]] = {}
        return

    def addObjs(self, srcs: tuple[str], target: str, rule: str) -> None:
        self.g.addNode(target)
        for s in srcs:
            self.g.addNode(s)
            self.g.addEdge(s, target)
        if target not in self.rules:
            self.rules[target] = []
        self.rules[target].append(rule)
        return

    def __resolve(self, changed: list[graph.Node]) -> list[graph.Node]:
        return graph.getOrder(self.g, changed)

    def doUpdate(self, changed: list[graph.Node]) -> None:
        for f in self.__resolve(changed):
            try:
                for r in self.rules[f]:
                    os.system(r)
            except KeyError:
                print(f"WARNING: no rule for {f}")
                pass
