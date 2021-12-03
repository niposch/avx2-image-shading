# Requirements

To run the vectorized version of this program, your cpu must support avx2.
Depending on the image used, quite a lot of ram may be required for this program to work properly.

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

Please place the input file at ./input.bmp
This program only supports bmp files.
The output file will be created at ./output.bmp

```
./program [staticThreadCount] [benchmarkRepetitionCount] [sleep time in milliseconds between benchmarks]
```
