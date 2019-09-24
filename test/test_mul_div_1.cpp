#include<assert.h>
int main(){
	int x, y;
	assert(2<x && x<4);  // [2, 4]
	assert(5<y && y<10);  // [5, 10]
	return x*y;          // [10, 40]
}
