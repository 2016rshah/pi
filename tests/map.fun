
fun addone(long a){
    return a + 1;
}

fun apply(funp action, long x) {
    return action(x);
}

fun main(){

    long array[3];
    array[0] = 10;
    array[1] = 11;
    array[2] = 12;
    long mapped[3];
    mapped[0] = apply(addone,array[0]);
    mapped[1] = apply(addone,array[1]);
    mapped[2] = apply(addone,array[2]);
    print mapped[0];
    print mapped[1];
    print mapped[2];
}


