g++ -g -c smt.cc -o smt.o
g++ -g -c main.cc -o main.o
g++ -g main.o smt.o -o main.out
rm *.o
