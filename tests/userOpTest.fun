define A long long a * b; #basic case: two integers, one operator
define B long long a * b / a - b; #more advanced case: two integers, many operators
define C long long var1 / var2 + var1; #variable names that are more than one letter long

struct point {
    long x;
    long y;
}

fun main() {
    long x = 10 A 10; #x = 100
    print(x);
    long y = x B x; #y = 0
    print(y)
    long z = arraytest();
    long q = structtest();
}

fun arraytest() { #tests array capabilities
    long array[4];
    array[0] = 2;
    array[1] = 3;
    array[2] = array[0] A array[1]; #6
    array[3] = array[2] B array[2]; #0
    print(array[0]);
    print(array[1]);
    print(array[2]);
    print(array[3]);
}

fun structtest() {
    point structpoint;
    structpoint.x = 10;
    structpoint.y = 1;
    print(structpoint.x C structpoint.y); #print 20
}


