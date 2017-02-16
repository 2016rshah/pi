fun potato(a, b, c){
	b = imsorry(c, b, a)
	q = printf(a)
	q = printf(b)
	q = printf(c)
	if(c < 30){
		q = potato(a, b, c + 1)
	}
	return a + b + c;
}

fun printf(data){
	print data
	return 0;
}
fun imsorry(x, y, z){
	return x + y * z;
}
fun greatadd(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, count){
	q = 23 * 12;
	if(count < 30000){
		qq = greatadd(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, count + 1);
	}
	return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20 + a21 + a22 + a23;
}
fun searchforinnerself(){
	if(depth < 240000000){
		depth = depth + 1;
		return searchforinnerself();
	}	
	return depth;
}
fun main(){
	x = potato(33, 33, 30)
	q = printf(x);
	l = greatadd(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0);
	mm = printf(l);
	q = printf(l == q);
	depth = 0;
	q = printf(searchforinnerself());
	ifyouneedhelpfeelfreetoemailmeatpersonfromturings2020atgmaildotcom = 1;
	}
