test:
	rm tests/*.c
	cp *.c tests/
	cp libglut.so.3 glut.h tests/
	$(MAKE) clean test -C tests/
