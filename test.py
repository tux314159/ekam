from ekam import *
g = DepGraph()
g.addObjs(("file1.c",), "file1.o", "gcc -o file1.o -c file1.c")
g.addObjs(("file2.c",), "file2.o", "gcc -o file2.o -c file2.c")
g.addObjs(("file1.o",), "out", "gcc -o out file1.o file2.o")
g.doUpdate(["file1.c"])
