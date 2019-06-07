Ovde se nalazi kompletno resenje, verzija koda koja radi (LLVM 6.0)

U direktorijumu test:
  gedit 3.cpp
  clang++-6.0 -O0 -emit-llvm 3.cpp -c
  
U glavnom direktorijumu:
  cmake CMakeLists.txt
  make
  opt-6.0 -instnamer -load src/AI_INTERVALI.so -AI-PROLAZ test/3.bc 

