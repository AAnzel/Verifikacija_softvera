int f(int x){
	int y = 7;
	return x - y;
}

int main(){
	int a = 6;
	int b = a + a;
	int c = f(a);

	return (c+b);
}
