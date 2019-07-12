#include<assert.h>
int g(int x){
	return x;
}

int f(int x){
	assert(x<99);
	return x;
}

int main(){
	int a = 74;
	return g(a);
}
