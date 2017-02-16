fun printtest() {
    print 1 + 2 + 3 + 4;
}

fun optest(x, y) {
    if(x == y) {print 0}
    if(x < y) {print 1}
    if(x > y) {print 2}
    if(x <> y) {print 3}
}

fun rectest(x, y, c, n) {
    if(n == c) {return x + y}
    else {
    	 return rectest(y, x + y, c + 1, n);;;
    }
}

fun main() {
    printtest()
    x = 999999999999999999999
    print x
    optest(1, 1)
    optest(0, 1)
    optest(1, 0)
    optest(0, 1)
    print rectest(0, 1, 0, 20)
}
