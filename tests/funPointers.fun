
fun add(long a){
    return a+1;
}

fun addten(long a){
    return a + 10;
}


fun apply(funp function,long a){
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
