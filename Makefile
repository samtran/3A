#NAME:Samantha Tran, Apurva Panse
#EMAIL: samantha.tran95@gmail.com
#ID: 804282884 504488023

default:
	gcc -o lab3a -g lab3a.c

clean:
	rm -r lab3b-504488023.tar.gz lab3a

dist:
	tar -zcvf lab3a-504488023.tar.gz ext2_fs.h README Makefile lab3a.c

