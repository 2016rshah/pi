
fun factorial(n) {
    print n
    i = 0
    v = 1

    while (i < n) {
        i = i + 1
        v = v * i
    }

    return v
}

fun main() {
    print factorial(0)
    print factorial(1)
    print factorial(2)
    print factorial(3)
    print factorial(4)
    print factorial(5)
    print factorial(6)
    print factorial(10)
}
