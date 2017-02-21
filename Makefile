test:
	rm tests/*.c
	cp *.c tests/
	$(MAKE) clean test -C tests/
