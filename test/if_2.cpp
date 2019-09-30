#include<assert.h>
int main(){
	
	int a;
	int b;
	int c; 

	if(a<100)
		b = 2;
	else
		b = 5; 

	// b should be in [2,2] U [5,5]

	assert(c < 15);
	assert(c > 5); // c should be in [5, 15]

	return b+c; // result shoud be [7, 17] U [10, 20]
}
