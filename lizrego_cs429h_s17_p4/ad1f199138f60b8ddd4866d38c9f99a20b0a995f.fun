fun side(x){
    print x * t
    t = t + 5
    return x
}

fun showthree(a,b,c){
    print a
    print b
    print c
    return a+b+c
}

fun main(){
    t = 10

    print showthree(side(1+0)+0, 2, 0+side(2938*0+3*1), side(10))
}