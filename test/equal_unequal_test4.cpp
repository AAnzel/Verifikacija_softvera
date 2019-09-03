#include<assert.h>

int main(){
	int x;
	assert(x>=7);  // Interval poverenja bice [6, +inf], tumaceno kao otvoreni interval, dakle 6 nije ukljuceno, vec je najmanji integer koji je moguc 7
	return x;
}
