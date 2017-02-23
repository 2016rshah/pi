
fun add(a){
    return a+1;
}

fun addten(a){
    return a + 10;
}


fun apply(function,a){
    return function(a);
}


fun main(){
    long var = 10;
    long some = apply(add,var);
    long more = apply(addten,var);
    print var;
    print some;
    print more;
}
