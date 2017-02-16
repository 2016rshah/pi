fun main(a1, a2, a3, a4, a5, a6, a7, a8) {
	if (a1 == 343) {
		print a7;
		return a8;
	}
	v = main2();
	v = fun3();
	v = 18446744073709551615 + 2; 
	a = rec(0, print5(5));
	print a;
	print v;
}

fun main2() 
	print main(343, 0, 0, 0, 0, 0, 434, 545);

fun fun3(a1) 
	print (a1 == a1 > a1 <> a1 < a1) + 18446744073709551615 + 1;

fun rec(v) {
	if(v<>100)
		return rec(v+1);
	return v;
}

fun print5(v) 
	print v;
