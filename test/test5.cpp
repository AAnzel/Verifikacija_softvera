int f(int x){
	int y = 7;
	return x - y;
}

int g(int x){
	return x+7;
}

int main(){
	int a = 6;
	int b = a + a;
	int c = g(a);

	return (c+b);
}
