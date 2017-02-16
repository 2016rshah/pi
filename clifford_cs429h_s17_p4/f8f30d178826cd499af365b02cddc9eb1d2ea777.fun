fun main(x, y, z) {
    print (x == x)
    print (y <> y)
    print (z > z) < (z == z)
    
    if(beenhere) {
        print x
        print y
        print z

    }
    else {
    x = f1()
    beenhere = 1;
    if(x == x) {
        print a
        y = f2(10)
        print y
        z = if0(0)
        print y
        print z
        print f2(3) + f2(4)
        print global(f2(5), f2(6))
        x = global(y,z,0,1,2,3,4,5,6,7)
        print x
        print y
        print z
        throwaway = main(z, y, x)

    }
    else
        print wrong
    }
    print throwaway == throwaway
}

fun f1();
fun f2(x) {
    if x == 0 return 0
    else if x == 1 return 1
    else return f2tail(1, x, 0, 1);
}
fun f2tail(x, n, a, b) {
    if (n == x) return b
    return f2tail(x + 1, n, b, a+b, extra1, extra2);
}

fun if0(y) {
    x = y + 1
    while(x < 10000000000) {
        x = x + f2(x)
    }
    print x
    return else1(x);
}

fun else1(x) {
    y = x + 1
    while(y < 17167680177650) {
       z = else1(y) + 1
       print z
    }
    return z;
}

fun global(t1, t2, dontneed1, dontneed2, dontneed3, dontneed4, dontneed5, dontneed6, dontneed7, dontneed8)
{
    
    return t1 * f2(t2)

}


