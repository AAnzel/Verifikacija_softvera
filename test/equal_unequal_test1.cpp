#include<assert.h>

int main(){
	int x;
	assert(x<5);  // Interval poverenja bice [-inf, 5], tumaceno kao otvoreni interval, dakle 5 nije ukljucena
	return x;
}
