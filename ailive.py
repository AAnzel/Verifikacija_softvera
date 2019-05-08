# Skript je pisan u Python 2.7
import os
import subprocess 

print("Dobrodosli u apstraktnu interpretaciju live :)")

PATH = "/home/student/Desktop/Verifikacija_Softvera"  # Konfigurisi ovo za svaku konkretnu primenu

code = open(os.path.join(PATH, "test/code.cpp"), "w")

code.write("int main(){\n")
print("\n\n==================================================================================")
print("korak 0: kreiraj svoj cpp fajl")
print("Unesi instrukcije Vaseg programa: [deklaracije, definicije, (, ), +, -, *, return]")
print("Unosi se do EOF.\n")
print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\nint main(){\n")
while True:
	try:
		linija = raw_input("\t")
		#print linija
		code.write("\t"+linija+"\n")  # upisi lepo formatirano
	except EOFError:
		print("\n}\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
		break

code.write("}\n")
code.close()

#print(os.path.abspath(os.curdir))
#subprocess.call("ls -l", shell=True)

# provera da li se fajl kompajlira
if os.system("g++ test/code.cpp") == 0:
	print("Kreirani kod se uspesno kompajlira.")
else:
	print("Kao sto vidis, tvoj uneti program ne prolazi kompilaciju.")
	print("Fatalna greska.")
	exit()

os.chdir("test")
print("\n\n==================================================================================")
print("korak 1: \"clang\" prevodi cpp fajl")
subprocess.call("clang-6.0 -load -O3 -emit-llvm code.cpp -c", shell=True)
os.chdir("../")
print("")

print("\n\n==================================================================================")
print("korak 2: alat \"opt\" vrsi apstraktnu interpretaciju: ")
subprocess.call("opt-6.0 -load src/AI_INTERVALI.so -AI-PROLAZ test/code.bc", shell=True)
print("")


