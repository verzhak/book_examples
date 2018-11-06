
all: kurs.c
	make -C /usr/src/linux/ SUBDIRS=`pwd` modules

clean:
	rm -f *.ko
	rm -f *.o
	rm -f *.mod.c
	rm -f *order
	rm -f *vers
	rm -Rf .??*
	rm -Rf *.markers

obj-m += kurs.o

