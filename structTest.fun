struct point{
	long x;
	long y;
}
struct line{
	point a;
	point b;
}
fun main(){
	point structpoint;
	line structline;
	long q;
	structpoint.x = 100;
	print structpoint.x;
        structline.a = structpoint;
        print structline.a.x;
	print 1
}
