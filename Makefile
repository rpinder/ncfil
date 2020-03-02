ncfil: ncfil.c
	gcc ncfil.c -lncurses -Wall -pedantic -std=c11 -o ncfil

install: ncfil
	cp $< /usr/local/bin/ncfil
