#include<assert.h>

int main(){
	int x;
	assert(x<=5);  // Interval poverenja bice [-inf, 6], tumaceno kao otvoreni interval, dakle 6 nije ukljuceno, vec je gornji integer koji je moguc 5
	return x;
}
