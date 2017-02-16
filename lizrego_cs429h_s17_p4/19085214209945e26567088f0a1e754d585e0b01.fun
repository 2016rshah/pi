fun fac1(curri,endi)
{
    if curri == endi return endi
    else { tmp = fac1(curri+1, endi)*curri; return tmp; }
}

fun fac2(endi)
{
    curri = 1
    ans = 1
    while(curri < (endi+1)) {
        ans = ans*curri
        curri = curri+1
    }
    return ans
}

fun funempty(endi) {

} fun funempty2(asdf,dasdfasdf) {
return dasdfasdf
}

fun funfun(funfun) return funfun;

fun fib(prev,curr,end) {
    prev = 1
    curr = 2
    term = 0
    while(term+2 < end) {
        tmp = prev + curr
        prev = curr
        curr = tmp
        term = term + 1
    }
    return curr
}

fun main() {
	x = main1(0)
}

fun main1(arg1) {
    fac1 = fac1(1,6)
    print fac1
    fac2 = fac2(6)
    print fac2
    print (fac1 == fac2)

    fac1 = fac1 == fac1(1,6,1)
    print fac1

    print fib(123,123,5)

    if(arg1 <> 0) {
        print arg1
    } else {
		asdfasdf=main1(fives(0,10)) {}
	}

    asdf1 = funempty(); asdf123123 = funempty2(1,2,3,4,5,6,1231231231223)

    funfun = 23
    print funfun(funfun+1)
}



fun fives(curri,endi) {
    five = 5
    curri = 0
    while curri < endi {
        five = five + 1
        print five
        curri = (curri+1)
    }
    return five;
}
