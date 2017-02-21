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
	long q = 0;
	structpoint.x = 100;
	print structpoint.x;
    structpoint.y = 200;
    print structpoint.y
    structline.a = structpoint;
    print structline.a.x;
	print 1
}
