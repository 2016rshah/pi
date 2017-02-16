fun main() {
	
	mov = 5
	jmp = 5
	and = 6
	var = 999;

	x = doubleprint(trip(double(mov), double(mov),  double(mov), double(mov), square(mov), square(mov), square(mov), square(mov)));

}

fun doubleprint(y) {
	print y
	print y
}

fun trip(a, b, c) {
	print a
	print b
	print c
	return a + b + c
}

fun double(x) {
	print x*2
	return x*2
}

fun square(x) {
	print x*x
	return x*x
}