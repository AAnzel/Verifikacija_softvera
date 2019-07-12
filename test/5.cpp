#include<assert.h>
int main(){
	int x, y;
	assert(x>10);        // [10, +inf]
	assert(5>y && 3<y);  // [3, 5]
	return x + y;        // [13, +inf]
}
