fun main(){
    long cases = 0;
    long current = getchar();
    while(current <> 10){
        cases = 10 * cases + (current - 48);
        current = getchar();
    }
    while(cases > 0){
        long value = 0;
        current = getchar();
        while(cases > 0){
            if((current > 47) & (current < 59)){
                value = 10 * value + (current - 48);
            }
            if(current == 10){
                print 6 * value;
                value = 0;
                cases = cases - 1;
            }
            current = getchar();
        }
    }
}
