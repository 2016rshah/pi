fun main() {
    long array[3];
    array[0] = 4;
    array[1] = 5;
    if (array[1] == 5) {
        array[2] = 7;
        long otherarray[4];
        otherarray[3] = array[1];
    }
    else array[2] = 6;
    print array[1]
    print array[2];
    print otherarray[3];
}
