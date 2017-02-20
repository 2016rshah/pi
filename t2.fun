
fun f(long a, long i, long n) {
    print a
    print i
    print n
    if (i == n) return a
    return f(a * i, i+1, n)
}

fun factorial(long n) return f(1,1,n)

fun main() {
    print factorial(5)
}
