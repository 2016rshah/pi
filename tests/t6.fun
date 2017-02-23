long z = 0;
fun b(long x, long y) {
    print x + y
}

fun c(long x,long y) {
    print x * y
}

fun a(long x, long y) {
    if (x > y) {
        long z = b(x,y)
    } else {
        long z = c(x,y)
    }
}

fun main() {
    long z = a(10,20)
    long z = a(20,10)
}

