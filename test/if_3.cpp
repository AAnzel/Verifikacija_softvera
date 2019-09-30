#include<assert.h>
int main(){
	
	int a;
	int b;
	int c; 
	
	if(a<15)
		b = 10;
	else
		b = 100;
	
	// b should be in [10, 10] U [100, 100]
	
	if(b>5)
		c = 5;
	else
		c = 20;
	// c should be in [5, 5] U [10, 10]
	return b - c; // result shoud be [5, 5] U [95, 95] U [-10, -10] U [80, 80]
}
