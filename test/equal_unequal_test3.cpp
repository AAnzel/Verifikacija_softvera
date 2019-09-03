#include<assert.h>

int main(){
	int x;
	assert(x>7);  // Interval poverenja bice [7, +inf], tumaceno kao otvoreni interval, dakle 7 nije ukljuceno, vec je najmanji integer koji je moguc 8
	return x;
}
