#NAME:
#EMAIL:
#ID:

default:
	gcc -o lab3a -g lab3a.c

clean:
	rm -r lab3b-504488023.tar.gz output.csv

dist:
	tar -zcvf lab3a-504488023.tar.gz README Makefile lab3a.c
