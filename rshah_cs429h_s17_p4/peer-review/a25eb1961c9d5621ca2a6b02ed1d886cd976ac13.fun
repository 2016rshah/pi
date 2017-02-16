fun main() {
	x = hello(1)
}

fun hello(x) {
	if (x<10){
		print(x)
		y = hello(x+1)
	}
}
