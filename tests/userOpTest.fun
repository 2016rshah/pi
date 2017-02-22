define A long long a * b; #basic case: two integers, one operator
define B long long a * b / a - b; #more advanced case: two integers, many operators

fun main() {
    long x = 10 A 10; #x = 100
    print(x);
    long y = x B x; #y = 0
    print(y)
}
