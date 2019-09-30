#include<assert.h>
int main(){
	
	int a;
	int b = 3; // b is in [3, 3]
	int c = 2;
	int d; 
	
	assert(a > 10); // a is in [10, +inf]
	
	if(a>100)
		d = b;
	else
		d = b+c;
	
	// d should be in [3, 3] U [5, 5]
	
	return d;
}
