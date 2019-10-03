#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>


#ifndef NUMT
#define NUMT	        16
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE		10000	
#endif

#ifndef NUMTRIES
#define NUMTRIES        100
#endif

#ifndef NUMTRIALS
#define NUMTRIALS		1000000
#endif



float	chooseRand(float, float);
void TimeOfDaySeed( );


int main() {
	#ifndef _OPENMP
		fprintf( stderr, "No OpenMP support!\n" );
		return 1;
	#endif

	const float XCMIN =	-1.0;
	const float XCMAX =	 1.0;
	const float YCMIN =	 0.0;
	const float YCMAX =	 2.0;
	const float RMIN  =	 0.5;
	const float RMAX  =	 2.0;

	TimeOfDaySeed( );		// seed the random number generator

	omp_set_num_threads( NUMT );	// set the number of threads to use in the for-loop:`
	
	// better to define these here so that the rand() calls don't get into the thread timing:
	float *xcs = new float [NUMTRIALS];
	float *ycs = new float [NUMTRIALS];
	float * rs = new float [NUMTRIALS];


	 // fill the random-value arrays:
     for( int n = 0; n < NUMTRIALS; n++ )
     {       
     	xcs[n] = chooseRand( XCMIN, XCMAX );
        ycs[n] = chooseRand( YCMIN, YCMAX );
     	rs[n] = chooseRand(  RMIN,  RMAX ); 
     }


     float maxPerformance = 0.;
     float currentProb;


     // looking for the maximum performance:
   	for( int t = 0; t < NUMTRIES; t++ )
    {
    	double time0 = omp_get_wtime( );

      	int numHits = 0;
			

		#pragma omp parallel for default(none) shared(xcs,ycs,rs) reduction(+:numHits)
		for( int n = 0; n < NUMTRIALS; n++ )
		{
			// randomize the location and radius of the circle:
			float xc = xcs[n];
			float yc = ycs[n];
			float  r =  rs[n];

			// solve for the intersection using the quadratic formula:
			float a = 2.;
			float b = -2.*( xc + yc );
			float c = xc*xc + yc*yc - r*r;
			float d = b*b - 4.*a*c;
			//If d is less than 0., then the circle was completely missed. (Case A) Continue on to the next trial in the for-loop.

			if (d >= 0) {
				// hits the circle:
				// get the first intersection:
				d = sqrt( d );
				float t1 = (-b + d ) / ( 2.*a );	// time to intersect the circle
				float t2 = (-b - d ) / ( 2.*a );	// time to intersect the circle
				float tmin = t1 < t2 ? t1 : t2;		// only care about the first intersection
			

				//If tmin is less than 0., then the circle completely engulfs the laser pointer. (Case B) Continue on to the next trial in the for-loop.
				if(tmin >= 0) {

					// where does it intersect the circle?
					float xcir = tmin;
					float ycir = tmin;

					// get the unitized normal vector at the point of intersection:
					float nx = xcir - xc;
					float ny = ycir - yc;
					float n = sqrt( nx*nx + ny*ny );
					nx /= n;	// unit vector
					ny /= n;	// unit vector

					// get the unitized incoming vector:
					float inx = xcir - 0.;
					float iny = ycir - 0.;
					float in = sqrt( inx*inx + iny*iny );
					inx /= in;	// unit vector
					iny /= in;	// unit vector

					// get the outgoing (bounced) vector:
					float dot = inx*nx + iny*ny;
					float outx = inx - 2.*nx*dot;	// angle of reflection = angle of incidence`
					float outy = iny - 2.*ny*dot;	// angle of reflection = angle of incidence`

					// find out if it hits the infinite plate:
					float t = ( 0. - ycir ) / outy;
					//If t is less than 0., then the reflected beam went up instead of down. Continue on to the next trial in the for-loop.
					//Otherwise, this beam hit the infinite plate. (Case D) Increment the number of hits and continue on to the next trial in the for-loop.
					if (t >= 0) {
						numHits++;
					}
				}
			}
		}



		double time1 = omp_get_wtime( );
		double megaTrialsPerSecond = (double)NUMTRIALS / ( time1 - time0 ) / 1000000.;
		if( megaTrialsPerSecond > maxPerformance )
			maxPerformance = megaTrialsPerSecond;
		currentProb = (float)numHits/(float)NUMTRIALS;
	}

	printf("Threads: %d\tTrials: %d\tProbability:  %8.2lf \n\t Peak Performance: %8.2lf Mega Trials/s\n\n", NUMT, NUMTRIALS, currentProb, maxPerformance);
	//printf("%8.2lf \t", maxPerformance);
	return 0;
}


float chooseRand( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

void TimeOfDaySeed( )
{
	struct tm y2k = { 0 };
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  timer;
	time( &timer );
	double seconds = difftime( timer, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}