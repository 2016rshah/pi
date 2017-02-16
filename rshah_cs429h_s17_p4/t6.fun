
fun b(x,y) {
    print x + y
}

fun c(x,y) {
    print x * y
}

fun a(x,y) {
    if (x > y) {
        z = b(x,y)
    } else {
        z = c(x,y)
    }
}

fun main() {
    z = a(10,20)
    z = a(20,10)
}

