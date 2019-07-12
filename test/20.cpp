#include<assert.h>
int main(){
	int a, b, c;
	assert(3<a && a<5);
	assert(30<b && b<50);
	c = a + b;
	assert(44>c && 40<c);
	return c;
}
