
fun f(a,i,n) {
    print a
    print i
    print n
    if (i == n) return a
    return f(a * i, i+1, n)
}

fun factorial(n) return f(1,1,n)

fun main() {
    print factorial(5)
}
