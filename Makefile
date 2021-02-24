#NAME: Aryaman Ladha, Akhil Vinta
#EMAIL: ladhaaryaman@ucla.edu, akhil.vinta@gmail.com
#ID: 805299802, 405288527

CC = gcc
CFLAGS = -g -Wall -Wextra

TAR = tar
TARFLAGS = -czvf
TAREXT = gz
submission-files = README Makefile lab3a.c ext2_fs.h

default: lab3a

clean:
	rm -f *.o lab3a-805299802.$(TAR).$(TAREXT) lab3a

dist: lab3a-805299802.tar.gz

lab3a-805299802.tar.gz: default
	$(TAR) $(TARFLAGS) $@ $(submission-files)

lab3a: lab3a.c
	$(CC) $(CFLAGS) lab3a.c -o $@