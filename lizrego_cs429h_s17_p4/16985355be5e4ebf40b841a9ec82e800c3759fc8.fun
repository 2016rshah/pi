fun hey(x, y) {
    print x
    print y
}

fun main() {
    if ((10 <> 7) == (1 > 0)) {
        print 20
    }

    if (10 <> 7 == 1 > 0) {
        print 20
    }

    a=10
    print a + 10 * 2
    if a > 100 print 10
    if a > 0 b = 10
    print b

    if a < 100 print 5
    if a < 5 print 6
    if a <> 11 print 7
    if a <> 10 print 2
    x = 199
    print x
    a = hey(50, 5 * (11))
    print x

    tail = 0;
}

fun tail() {
    if (tail > 20000000) return tail
    else tail = tail + 1
    return tail()
}
