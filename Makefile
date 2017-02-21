transfer:
	rm tests/*.c
	cp *.c tests/
	cp libglut.so.3 glut.h tests/

clean:
	$(MAKE) transfer
	$(MAKE) clean -C tests/

test:
	$(MAKE) transfer
	$(MAKE) test -C tests/

error:
	$(MAKE) transfer
	$(MAKE) error -C tests/

graphics:
	$(MAKE) transfer
	$(MAKE) graphics -C tests/

all:
	$(MAKE) transfer
	$(MAKE) all -C tests/
