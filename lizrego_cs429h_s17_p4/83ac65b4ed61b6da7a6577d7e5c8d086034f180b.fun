fun multiply(n, x) {
	return mult(n,x,n)
}

fun mult(n,x, start) {
	if((n+1 < start+start) == 0) {
		return x
	}
	return x+mult(n+1,x, start)
}

fun power(x,n) {
	return powers(x,n,n)
}

fun powers(x,n,start) {
	if((n+1<start+start) == 0) {
		return x
	}
	return multiply(x, powers(x, n+1, start))
}

fun main() {
	print multiply(20,50)
	print power(2, 11)
}
