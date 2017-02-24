TESTS=$(sort $(wildcard *.pi))
ETESTS=$(sort $(wildcard *.error))
GTESTS=$(sort $(wildcard *.graphics))
ITESTS=$(sort $(wildcard *.io))
PROGS=$(subst .pi,,$(TESTS))
EPROGS=$(subst .error,,$(ETESTS))
GPROGS=$(subst .graphics,,$(GTESTS))
IPROGS=$(subst .io,,$(ITESTS))
OUTS=$(patsubst %.pi,%.out,$(TESTS))
EOUTS=$(patsubst %.error,%.out,$(ETESTS))
GOUTS=$(patsubst %.graphics,%.out,$(GTESTS))
IOUTS=$(patsubst %.io,%.out,$(ITESTS))
DIFFS=$(patsubst %.pi,%.diff,$(TESTS))
EDIFFS=$(patsubst %.error,%.diff,$(ETESTS))
GDIFFS=$(patsubst %.graphics,%.diff,$(GTESTS))
IDIFFS=$(patsubst %.io,%.diff,$(ITESTS))
RESULTS=$(patsubst %.pi,%.result,$(TESTS))

CFILES=$(sort $(wildcard *.c))
OFILES=$(patsubst %.c,%.o,$(CFILES))

.SECONDARY:

.PROCIOUS : %.o %.S %.out
CFLAGS=-g -std=gnu99 -O0 -Werror -Wall

p5 : $(OFILES) Makefile
	gcc $(CFLAGS) -o p5 $(OFILES) -lGL -lGLU libglut.so.3 -lm

$(OFILES) : %.o : %.c Makefile
	gcc $(CFLAGS) -MD -c $*.c -I .

%.o : %.S Makefile
	gcc -MD -c $*.S

%.S : %.error p5
	@echo "========= error test $* ========="
	./p5 < $*.error> $*.S

%.S : %.pi p5
	@echo "========== $* =========="
	./p5 < $*.pi > $*.S

%.S : %.graphics p5
	@echo "========== graphics $* ==========="
	./p5 < $*.graphics > $*.S

%.S : %.io p5
	@echo "========== io $* =========="
	./p5 < $*.io > $*.S


eprogs : $(EPROGS)

$(EPROGS) : % : %.o
	gcc -o $@ $*.o graphicfuncs.o -lGL -lGLU libglut.so.3 playSound.o -lm

gprogs : $(GPROGS)

$(GPROGS) : % : %.o
	gcc -o $@ $*.o graphicfuncs.o -lGL -lGLU libglut.so.3 playSound.o -lm

iprogs : $(IPROGS)

$(IPROGS) : % : %.o
	gcc -o $@ $*.o graphicfuncs.o -lGL -lGLU libglut.so.3 playSound.o -lm

progs : $(PROGS)

$(PROGS) : % : %.o
	gcc -o $@ $*.o graphicfuncs.o -lGL -lGLU libglut.so.3 playSound.o -lm

outs : $(OUTS)

$(OUTS) : %.out : %
	./$* > $*.out

eouts: $(EOUTS)

$(EOUTS) : %.out : %
	./$* > $*.out

gouts : $(GOUTS)

$(GOUTS) : %.out : %
	./$*

iouts: $(IOUTS)

$(IOUTS) : %.out : %
	./$*

diffs : $(DIFFS)

$(DIFFS) : %.diff : Makefile %.out %.ok
	@(((diff -b $*.ok $*.out > /dev/null 2>&1) && (echo "===> $* ... pass")) || (echo "===> $* ... fail" ; echo "----- expected ------"; cat $*.ok ; echo "----- found -----"; cat $*.out)) > $*.diff 2>&1

ediffs : $(EDIFFS)

$(EDIFFS) : %.diff : Makefile %.out

$(GDIFFS) : %.diff : Makefile %.out

$(IDIFFS) : %.diff : Makefile %.out

$(RESULTS) : %.result : Makefile %.diff
	@cat $*.diff

test : Makefile $(DIFFS)
	@cat $(DIFFS)

error: Makefile $(EDIFFS)

graphics: Makefile $(GDIFFS)

io: Makefile $(IDIFFS)

all: 
	make test error graphics io

clean :
	rm -f $(PROGS)
	rm -f $(EPROGS)
	rm -f $(GPROGS)
	rm -f $(IPROGS)
	rm -f *.S
	rm -f *.out
	rm -f *.d
	rm -f *.o
	rm -f p5
	rm -f *.diff

-include *.d
