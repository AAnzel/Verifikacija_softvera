#include<assert.h>
int main(){
	
	int a;
	int b;
	int c; 
	
	if(a>100)
		b = 1;
	else
		b = 5;
	
	// b should be in [1, 1] U [5, 5]
	
	if(a>5)
		c = -10;
	else
		c = 10;

	// c should be in [-10, -10] U [10, 10]

	return b * c; // result shoud be [-10, -10] U [-50, -50] U [10, 10] U [50, 50]
}
