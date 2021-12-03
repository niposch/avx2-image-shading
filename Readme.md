# How to compile

## Windows

```
# vectorized program
    g++ ./main.cpp -march=native -o withavx.exe -O0 -std=c++11

# normal program
    g++ ./main.cpp -march=core2 -o withoutavx.exe -O0 -std=c++11
```

## Linux

```
# vectorized program
    g++ ./main.cpp -march=native -o withavx -O0 -std=c++11 -lpthread

# normal program
    g++ ./main.cpp -march=core2 -o withoutavx -O0 -std=c++11 -lpthread
```

# How to run the program

```
./program [staticThreadCount] [benchmarkRepetitionCount] [sleep time in milliseconds between benchmarks]
```
