student@ubuntu:~/Desktop/Verifikacija_Softvera/test$ cat 1.cpp
int main () {
   int k = 42;
   int a = k + k;
   int d = (a + a) - (a - a);
   return (d + a);
}
student@ubuntu:~/Desktop/Verifikacija_Softvera/test$ cat 2.cpp
int main () {
   int k = 987;
   int a = k + k;
   return a;
}

student@ubuntu:~/Desktop/Verifikacija_Softvera/test$ cat 3.cpp
int main(){
	int x = 17;
	return x + x;
}
student@ubuntu:~/Desktop/Verifikacija_Softvera/test$ cat 4.cpp
int main(){
	int a = 3;
	int b = a + a;
	int c = b + a;
	int d = c + a;
	int e = d - 5;
	return e-5;
}
