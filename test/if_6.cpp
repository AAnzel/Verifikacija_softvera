#include<assert.h>
int main(){

	int a;
	int b; 
	int c;

	if(a > 0)
		b = 2;
	else
		b = 10;

	// b should be in [2, 2] U [10, 10]
	
	assert(c < 10);
	assert(c > 2);

	// c should be in [2, 10]

	return b/c; // result should be [0, 1] U [1, 5]

}
