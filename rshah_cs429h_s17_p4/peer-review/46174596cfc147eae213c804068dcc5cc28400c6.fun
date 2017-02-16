fun main() {
	print f1(1)
}

fun f1(z) {
	if z == 4 return 1;
	return f1(z+1) + f1(z + 1);
}
