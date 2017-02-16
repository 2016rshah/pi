fun a(x){
	x = x+1
	return b(x)
}

fun b(x) {
	if x > 10
		print x
	else if x <> 10
		print x+1000000
	if x == 15
		if x == 10 x =00000
		else return 12345678910111213141516
	return a(x)
}

fun fib(i, n) {
	if i>n return 0
	if i==n return 1
	if i+1==n return 1
	return fib(i+2, n) + fib(i+1, n)
}

fun gimmeargs(a1,b1,c1,d1,e1,f1,g1,h1,i1,j1,k1,l1,m1,n1,o1,p1,q1,r1,s1,t1,u1,v1,w1,x1,y1,z1, a2,b2,c2,d2,e2,f2,g2,h2,i2,j2,k2,l2,m2,n2,o2,p2,q2,r2,s2,t2,u2,v2,w2,x2,y2,z2, a3,b3,c3,d3,e3,f3,g3,h3,i3,j3,k3,l3,m3,n3,o3,p3,q3,r3,s3,t3,u3,v3,w3,x3,y3,z3, a4,b4,c4,d4,e4,f4,g4,h4,i4,j4,k4,l4,m4,n4,o4,p4,q4,r4,s4,t4,u4,v4,w4,x4,y4,z4) {
	return a1*b1*c1*d1*e1*f1*g1*h1*i1*j1*k1*l1*m1*n1*o1*p1*q1*r1*s1*t1*u1*v1*w1*x1*y1*z1+ a2*b2*c2*d2*e2*f2*g2*h2*i2*j2*k2*l2*m2*n2*o2*p2*q2*r2*s2*t2*u2*v2*w2*x2*y2*z2+ a3*b3*c3*d3*e3*f3*g3*h3*i3*j3*k3*l3*m3*n3*o3*p3*q3*r3*s3*t3*u3*v3*w3*x3*y3*z3+ a4*b4*c4*d4*e4*f4*g4*h4*i4*j4*k4*l4*m4*n4*o4*p4*q4*r4*s4*t4*u4*v4*w4*x4*y4*z4;
}
fun main() {
	x = 10007
	print x
	print a(7)
	print x
	print fib(1, 40)
    print gimmeargs(1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4)
}
