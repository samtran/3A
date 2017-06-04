#NAME: Samantha Tran Apurva Panse 
#EMAIL: samantha.tran95@gmail.com
#ID: 804282884 504488023

default:
	gcc -o lab3a -g lab3a.c

clean:
	rm -r lab3b-804282884.tar.gz output.csv

dist:
	tar -zcvf lab3a-804282884.tar.gz README Makefile lab3a.c ext2_fs.h
