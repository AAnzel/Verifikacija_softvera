#include<assert.h>
int funkcija(int x){
	assert(-1995<x && x<1995);
	return x;
}

int main(){
	int a;
	return funkcija(a);
}
