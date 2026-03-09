CFLAGS=-std=gnu99 -Wall

libext2fsal:  e2fs.o ext2fsal.o ext2fsal_cp.o ext2fsal_rm.o ext2fsal_ln_hl.o ext2fsal_ln_sl.o ext2fsal_mkdir.o
	gcc $(CFLAGS) -shared -fPIC -o libext2fsal.so $^
	cp libext2fsal.so ../bin	

%.o : %.c ext2.h e2fs.h
	gcc $(CFLAGS) -g -c -fPIC $<

test : test.c e2fs.c ext2.h e2fs.h ext2fsal_*.c ext2fsal.h
	gcc -o test ext2fsal_*.c test.c e2fs.c

clean : 
	rm -f *.o libext2fsal.so *~