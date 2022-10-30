fn int wacky_function(int a, uint b, float c, int d, float e, int num) {
    int temp_var = 0;
    for (int i = 0; i < num; i++) {
        temp_var += (a + b)*c+d/e;
    }
    return temp_var;
}

#fn void second_wacky_Function(int a, uint b, float c, int d, float e, float & result, int num) {for(int i=0;i<num;i++){result*=(a+b)*c+d/e;}}

fn void main() {
    int a = -10;
    uint b = 20;
    float c = 30.543;
    int d=40;
    float e=-50;
    float result;

    result=wacky_function(a, b, c, d, e, 10); #result should be 3040
    #second_wacky_Function(a, b, c, d, e, result, result/58.4) #result should be 18880.8
}