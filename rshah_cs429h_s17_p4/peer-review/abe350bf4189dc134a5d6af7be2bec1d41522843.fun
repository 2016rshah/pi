fun recurse(a, i) {
	if(a == i) return a;
	a = a + 1;
	return recurse(a, i) + recurse(a,i);
}

fun sadness(one, two) {
	return one * one * one * one * two + two;
}

fun main() {
	print recurse(0, 4);
	print sadness(10, 25, 2000000, 60, 10, 18, 9);
}
