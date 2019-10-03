#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "simd.p4.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE		10000	
#endif

#ifndef NUMTRIES
#define NUMTRIES        10
#endif

void arrayFill(float *array, int arraySize);

float A[ARRAYSIZE];
float B[ARRAYSIZE];
float C[ARRAYSIZE];

int main()
{
	arrayFill(A, ARRAYSIZE);
	arrayFill(B, ARRAYSIZE);


	
	double part1_time = 0;
	float part1_peak_time = 100;
	double part2_time = 0;
	float part2_peak_time = 100;
	double part3_time = 0;
	float part3_peak_time = 100;
	double part4_time = 0;
	float part4_peak_time = 100;


	//SIMD array multiplication
	for(int i = 0; i < NUMTRIES; i++) 
	{
		double time_start = omp_get_wtime();
		SimdMul(A, B, C, ARRAYSIZE);
		double time_end = omp_get_wtime();

		part1_time = (double)time_end - time_start;

		if((double)part1_time < part1_peak_time)
		{
			part1_peak_time = part1_time;
		}
	}


	//CPP array multiplication
	for(int i = 0; i < NUMTRIES; i++)
	{
		double time_start = omp_get_wtime();

		for(int i = 0; i < ARRAYSIZE; i++)
		{
			C[i] = A[i] + B[i];
		}

		 double time_end = omp_get_wtime();



		part2_time = (double)time_end - time_start;

		if((double)part2_time < part2_peak_time) 
		{
			part2_peak_time = part2_time;
		}
	}

	
	//SIMD array multiplication and reduction
	for(int i = 0; i < NUMTRIES; i++) 
	{
		double time_start = omp_get_wtime();
		SimdMulSum(A, B, ARRAYSIZE);
		double time_end = omp_get_wtime();



		part3_time = (double)time_end - time_start;

		if((double)part3_time < part3_peak_time)
		{
			part3_peak_time = part3_time;
		}
	}


	//CPP array multiplication and reduction
	float sum = 0.;

	for(int i = 0; i < NUMTRIES; i++)
	{
		double time_start = omp_get_wtime();

		for( int i = 0; i < ARRAYSIZE; i++ )
		{
			sum += A[i] * A[i];
		}

		double time_end = omp_get_wtime();



		part4_time =(double)time_end - time_start;

		if((double)part4_time < part4_peak_time) 
		{
			part4_peak_time = part4_time;
		}
	}
	

	//Speedup for array multiplication
	float speedup1 = (double)part2_peak_time/part1_peak_time;

	//Speedup for array multiplication and reduction
	float speedup2 = (double)part4_peak_time/part3_peak_time;

	printf("%d\t%8.2lf\t%8.2lf", ARRAYSIZE, speedup1, speedup2);

	return 0;
}



void arrayFill(float *array, int arraySize) {
	srand(omp_get_wtime());

	for (int i = 0; i < arraySize; i++)
		array[i] = (float)(rand() % 10000);
}