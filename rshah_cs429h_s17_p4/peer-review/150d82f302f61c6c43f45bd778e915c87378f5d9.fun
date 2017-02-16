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

fun main() {
    fac1 = fac1(1,6)
    print fac1
    fac2 = fac2(6)
    print fac2
    print (fac1 == fac2)
}
