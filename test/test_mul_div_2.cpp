#include<assert.h>
int main(){
	int x, y;
	assert(4<x && x<6);  // [4, 6]
	assert(3<y && y<5);  // [3, 5]
	return x/y;          // [0, 2]
}
