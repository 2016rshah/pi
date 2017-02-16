fun main() {
	print 18446744073709551615 > 0
	print 18446744073709551615 < 0

	printf = 1
	print printf
	print printf()

	print a(a(2, a(3, 4, 5), 1), a(a(1, 2, 3), 1, a(3, 3, 3)), a(0, 1, 2))

	n = r(0)
}

fun printf() {
	return 2
}

fun a(b, c, d) {
	return b + c + d
}

fun r(x) {
	print x
	if (x <> 10)
	n = r(x + 1)
}