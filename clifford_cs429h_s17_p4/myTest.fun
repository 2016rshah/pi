fun stupid(x){
print 1
if(x > 100){
	return 1;
}
return 1 + stupid(x + 1);
}
fun main(){
q = 1 + stupid(0);
print q
}
