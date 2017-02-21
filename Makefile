test:
	rm tests/p5.c
	cp p5.c tests/
	$(MAKE) clean test -C tests/
