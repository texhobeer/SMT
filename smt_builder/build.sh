g++ -O4 -c smt.cc -o smt.o -std=c++11
g++ -O4 -c main.cc -o main.o -std=c++11
g++ -O4 main.o smt.o -o main.out -std=c++11
rm *.o
