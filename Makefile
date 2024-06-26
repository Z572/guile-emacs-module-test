
CC=gcc
CFLAGS=`pkg-config --cflags guile-3.0`
LIBS=`pkg-config --libs guile-3.0`
module.so: module.c
	${CC} -shared -fpic $< ${LIBS} ${CFLAGS} -o $@

run: module.so
	emacs --eval "(progn (module-load (expand-file-name \"module.so\")))"
