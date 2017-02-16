fun x(y, z) {
	y = y+1
	print y
	print z
	x = y
	return x
}

fun main() {
	print x(1, 5)
}

