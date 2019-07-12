#include<assert.h>
int main(){
	int x, y;
	assert(3<x && x<6);  // [3, 6]
	assert(4<y && y<7);  // [4, 7]
	return x-y;          // [-4, 2]
}
