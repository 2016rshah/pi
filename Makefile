TESTS=$(sort $(wildcard *.fun))
PROGS=$(subst .fun,,$(TESTS))
OUTS=$(patsubst %.fun,%.out,$(TESTS))
DIFFS=$(patsubst %.fun,%.diff,$(TESTS))
RESULTS=$(patsubst %.fun,%.result,$(TESTS))

CFILES=$(sort $(wildcard *.c))
OFILES=$(patsubst %.c,%.o,$(CFILES))

.SECONDARY:

.PROCIOUS : %.o %.S %.out
CFLAGS=-g -std=gnu99 -O0 -Werror -Wall

p5 : $(OFILES) Makefile
	gcc $(CFLAGS) -o p5 $(OFILES) -lGL -lGLU libglut.so.3

$(OFILES) : %.o : %.c Makefile
	gcc $(CFLAGS) -MD -c $*.c -I .

%.o : %.S Makefile
	gcc -MD -c $*.S

%.S : %.fun p5
	@echo "========== $* =========="
	./p5 < $*.fun > $*.S

progs : $(PROGS)

$(PROGS) : % : %.o
	gcc -o $@ $*.o graphicfuncs.o -lGL -lGLU libglut.so.3 playSound.o

outs : $(OUTS)

$(OUTS) : %.out : %
	./$* > $*.out

diffs : $(DIFFS)

$(DIFFS) : %.diff : Makefile %.out %.ok
	@(((diff -b $*.ok $*.out > /dev/null 2>&1) && (echo "===> $* ... pass")) || (echo "===> $* ... fail" ; echo "----- expected ------"; cat $*.ok ; echo "----- found -----"; cat $*.out)) > $*.diff 2>&1

$(RESULTS) : %.result : Makefile %.diff
	@cat $*.diff

test : Makefile $(DIFFS)
	@cat $(DIFFS)

clean :
	rm -f $(PROGS)
	rm -f *.S
	rm -f *.out
	rm -f *.d
	rm -f *.o
	rm -f p5
	rm -f *.diff

-include *.d
