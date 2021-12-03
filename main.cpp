#include <stdio.h>
#include <immintrin.h>
#include <algorithm>
#include <thread>
#include <vector>
#include <omp.h>

// used as reference: https://db.in.tum.de/~finis/x86-intrin-cheatsheet-v2.1.pdf


// function cited from https://stackoverflow.com/a/9296467/15191853 at 8:00 pm 31/10/2021
// it was however heavily modified
void readBMP(char* filename, unsigned char * output)
{
    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];

    // read the 54-byte header
    fread(info, sizeof(unsigned char), 54, f); 

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    // allocate 3 bytes per pixel
    long long int size = 3 * width * height;

    // read the rest of the data at once
    fread(output, sizeof(unsigned char), size, f); 
    fclose(f);
}
// end of function citation

// inspired by the readBMP function
void writeBMP(char* filename, char* originalFile, unsigned char* bmpData){
    int i;
    FILE* origFile = fopen(originalFile, "rb");
    FILE* newFile = fopen(filename, "wb");
    unsigned char info[54];

    // read the 54-byte header
    fread(info, sizeof(unsigned char), 54, origFile); 
    fwrite(info, sizeof(unsigned char), 54, newFile);

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];


    long long int size = 3 * width * height;
    // write the rest of the data at once
    fwrite(bmpData, sizeof(unsigned char), size, newFile); 
    fclose(origFile);
    fclose(newFile);
}
#define DARKEN_VALUE 60
#define FILE_NAME "./input.bmp"
#define THREADED true

#if __AVX2__
__m256i darkenVector = _mm256_set1_epi8((unsigned char) DARKEN_VALUE);
#endif


typedef struct pixel32Args_t {
    unsigned char* input;
    unsigned char* output;
} pixel32Args_t;

void modify32Pixel(unsigned char * input, unsigned char * output){
    #if __AVX2__
    __m256i input1 = _mm256_loadu_si256((__m256i*) input);
    __m256i darker = _mm256_subs_epu8(input1, darkenVector);
    _mm256_storeu_si256((__m256i*) output, darker);
    #else
    for (long long int i = 0; i < 32; i++)
    {
        output[i] = std::max(input[i] - DARKEN_VALUE, 0);
    }
    
    #endif
}
long long int getDataLength(char * filename){
    FILE* origFile = fopen(filename, "rb");
    unsigned char info[54];
    // read the 54-byte header
    fread(info, sizeof(unsigned char), 54, origFile); 
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    return 3 * width * height;
}

void runXTimes(unsigned char* rawImage, unsigned char* output, long long dataLengthAndPadding, int calculationCount){
    for (int i = 0; i < calculationCount; i++)
    {
        for (long long int j = 0; j < dataLengthAndPadding; j = j + 32)
        {
            modify32Pixel(&rawImage[j], &output[j]);
        }
    }
}
void run(int threadCount, unsigned char* input, unsigned char* output, unsigned long long dataLength, unsigned long long padding){
    int numberOfIterations = 512; // smallest number divisible by 1 to 10
    std::vector<std::thread> threads;
    for (size_t i = 0; i < threadCount; i++)
    {
        threads.push_back(std::thread(runXTimes, input, output, dataLength+padding,numberOfIterations/threadCount));

        printf("started thread %d\n",i);
    }

    for (size_t i = 0; i < threadCount; i++)
    {
        threads[i].join();
        printf("thread %d ended\n",i);
    }
}
/* ;*/
int main(int argc, char ** args){
    long long int dataLength = getDataLength(FILE_NAME);
    int padding = (32 - (dataLength % 32))%32;  // add padding to be able to only process 32 long chunks

    int staticThreadCount = -1;
    int repetitionsCount = 8;
    int waitBetweenReps = 4000; // wait 4 seconds between runs to isolate power consumption spikes
    printf("Developed by niposch in 2021.\n");
    printf("you can run the program via ./program.exe [staticThreadCount] [repetitions] [waitBetweenReps (in Milliseconds)], you can configure parameters relating to the tests performed.\n");
    printf("Some parameters have -1 as their default value.\n");
    printf("if staticThreadCount is set to -1, it will perform sequential tests with 1,2,4 and 8 threads.\n");
    printf("\n!!!the program only supports 1,2,4,8 or 10 threads!!!\n\n");
    printf("The supplied values are not thorouhgly validated. It is not guranteed that the program actually supports any value value inserted into thread count. Possible values are 1,2,4,8,10. This is the case because the multithreading is somewhat basic in its implementation.\n");
    printf("RepetitionsCount can not be modified if staticThreadCount is set to -1!\n");
    if(argc >= 2){
        int staticThreadCountInput = atoi(args[1]);
        if(staticThreadCountInput<1 && staticThreadCountInput != -1){
            if(staticThreadCountInput != 0){
                printf("Running test with default value for staticThreadCount as it was out of range\n");
            }
            staticThreadCount = -1;
        }
        else{
            staticThreadCount = staticThreadCountInput;
            printf("Running test with static thread count supplied.\n");
        }
    }
    if(argc >= 3){
        printf("RepetitionsCount was supplied\n");
        repetitionsCount = atoi(args[2]);
        if(repetitionsCount < 1){
            printf("RepetitionsCount was out of range and reset to its default value.\n");
            repetitionsCount = 8;
        }
        if(staticThreadCount == -1){
            printf("RepetitionsCount cannot be modified if the staticThreadCount is set on its default value!!! The RepetitionsCount is reset to its default value.\n");
            repetitionsCount = 8;
        }
    }
    if(argc >= 4){
        waitBetweenReps = atoi(args[3]);
        if(waitBetweenReps <0){
            printf("Wait between reps is out of range! It was reset to its default value.\n");
            waitBetweenReps = 4000;
        }
    }
    printf("\nstaticThreadCount = %d\n", staticThreadCount);
    printf("repetitionsCount = %d\n", repetitionsCount);
    printf("waitBetweenReps = %d ms\n", waitBetweenReps);
    #if __AVX2__
    printf("This program was compiled with AVX2 enabled. To run it without AVX2, recompile it with the non avx gcc command.\n");
    #else
    printf("This program was compiled with AVX2 disabled. To run it with AVX2, recompile it with the avx2 gcc command.\n");
    #endif
    printf("=========================\n\n");
    unsigned char* rawImage = new unsigned char[dataLength + padding];
    readBMP(FILE_NAME, &rawImage[0]);

    unsigned char* output = new unsigned char[dataLength + padding];

    for (size_t i = 1; i <=repetitionsCount ; )
    {
        int threadCount = 0;
        if(staticThreadCount < 1){
            threadCount = i;
        }
        else{
            threadCount = staticThreadCount;
        }
        auto start = std::chrono::high_resolution_clock::now();
        printf("\n \n started run with %d threads\n", threadCount);
        run(threadCount, rawImage, output, dataLength, padding);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        printf("finished run with %d threads. It took %f sec", threadCount, elapsed.count());
        std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenReps));
        if(staticThreadCount == -1){
            i = i*2;
        }
        else{
            i = i + 1;
        }
    }
    writeBMP("output.bmp", FILE_NAME, &output[0]);
    free(rawImage);
    free(output);
    return 0;
}
