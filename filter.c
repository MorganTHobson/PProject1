/*******************************************************************************
*
*  Filter a large array based on the values in a second array.
*
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <omp.h>
#include <math.h>
#include <assert.h>

/* Example filter sizes */
#define DATA_LEN  512*512*256
#define FILTER_LEN  1024
#define THREADS 8
#define UNROLL 16

/* Subtract the `struct timeval' values X and Y,
    storing the result in RESULT.
    Return 1 if the difference is negative, otherwise 0. */
int timeval_subtract (struct timeval * result, struct timeval * x, struct timeval * y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }
    
  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

/* Function to apply the filter with the filter list in the outside loop */
void serialFilterFirst ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp )
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the filter */ 
  for (int y=0; y<filter_len; y++) { 
    /* for all elements in the data */
    for (int x=0; x<data_len; x++) {
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Serial filter first took %lu seconds and %lu microseconds.  Filter length = %d\n", tresult.tv_sec, tresult.tv_usec, filter_len );
  fprintf (fp, "%d,%lu,%lu\n", filter_len, tresult.tv_sec, tresult.tv_usec);
}


/* Function to apply the filter with the filter list in the outside loop */
void serialDataFirst ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp )
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y++) { 
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Serial data first took %lu seconds and %lu microseconds.  Filter length = %d\n", tresult.tv_sec, tresult.tv_usec, filter_len );
  fprintf (fp, "%d,%lu,%lu\n", filter_len, tresult.tv_sec, tresult.tv_usec);
}

/* Function to apply the filter with the filter list in the outside loop */
void parallelFilterFirst ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp, int threads)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the filter */ 
  #pragma omp parallel for
  for (int y=0; y<filter_len; y++) { 
    /* for all elements in the data */
    for (int x=0; x<data_len; x++) {
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel filter first took %lu seconds and %lu microseconds.  Threads = %d\n", tresult.tv_sec, tresult.tv_usec, threads );
  fprintf (fp, "%d,%lu,%lu\n", threads, tresult.tv_sec, tresult.tv_usec);
}



/* Function to apply the filter with the filter list in the outside loop */
void parallelDataFirst ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp, int threads)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  #pragma omp parallel for
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y++) { 
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel data first took %lu seconds and %lu microseconds.  Threads = %d\n", tresult.tv_sec, tresult.tv_usec, threads );
  fprintf (fp, "%d,%lu,%lu\n", threads, tresult.tv_sec, tresult.tv_usec);
}

/* Unrolled version of the parallel data first function */
void parallelDataFirstUnrolled ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  #pragma omp parallel for
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y+=UNROLL) { 
      /* check if the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        output_array[x] = input_array[x];
      }
      else if (input_array[x] == filter_list[y+1]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+2]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+3]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+4]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+5]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+6]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+7]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+8]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+9]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+10]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+11]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+12]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+13]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+14]) {
        output_array[x] = input_array[x];
      }
      if (input_array[x] == filter_list[y+15]) {
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel data first unrolled took %lu seconds and %lu microseconds.  Unroll = %d\n", tresult.tv_sec, tresult.tv_usec, UNROLL);
  fprintf (fp, "%d,%lu,%lu\n", UNROLL, tresult.tv_sec, tresult.tv_usec);
}

void parallelDataFirstScheduledDynamic ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* Scheduling label */
  char *schedule = "dynamic";

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  #pragma omp parallel for schedule(dynamic)
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y++) { 
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel data first scheduled took %lu seconds and %lu microseconds.  Scheduling = %s\n", tresult.tv_sec, tresult.tv_usec, schedule );
  fprintf (fp, "%s,%lu,%lu\n", schedule, tresult.tv_sec, tresult.tv_usec);
}

void parallelDataFirstScheduledGuided ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* Scheduling label */
  char *schedule = "guided";

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  #pragma omp parallel for schedule(guided)
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y++) { 
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel data first scheduled took %lu seconds and %lu microseconds.  Scheduling = %s\n", tresult.tv_sec, tresult.tv_usec, schedule );
  fprintf (fp, "%s,%lu,%lu\n", schedule, tresult.tv_sec, tresult.tv_usec);
}


void parallelDataFirstScheduledStatic32 ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* Scheduling label */
  char *schedule = "static32";

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  #pragma omp parallel for schedule(static, 32)
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y++) { 
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel data first scheduled took %lu seconds and %lu microseconds.  Scheduling = %s\n", tresult.tv_sec, tresult.tv_usec, schedule );
  fprintf (fp, "%s,%lu,%lu\n", schedule, tresult.tv_sec, tresult.tv_usec);
}


void parallelDataFirstScheduledStatic64 ( int data_len, unsigned int* input_array, unsigned int* output_array, int filter_len, unsigned int* filter_list, FILE *fp)
{
  /* Variables for timing */
  struct timeval ta, tb, tresult;

  /* Scheduling label */
  char *schedule = "static64";

  /* get initial time */
  gettimeofday ( &ta, NULL );

  /* for all elements in the data */
  #pragma omp parallel for schedule(static, 64)
  for (int x=0; x<data_len; x++) {
    /* for all elements in the filter */ 
    for (int y=0; y<filter_len; y++) { 
      /* it the data element matches the filter */ 
      if (input_array[x] == filter_list[y]) {
        /* include it in the output */
        output_array[x] = input_array[x];
      }
    }
  }

  /* get initial time */
  gettimeofday ( &tb, NULL );

  timeval_subtract ( &tresult, &tb, &ta );

  printf ("Parallel data first scheduled took %lu seconds and %lu microseconds.  Scheduling = %s\n", tresult.tv_sec, tresult.tv_usec, schedule );
  fprintf (fp, "%s,%lu,%lu\n", schedule, tresult.tv_sec, tresult.tv_usec);
}


void checkData ( unsigned int * serialarray, unsigned int * parallelarray )
{
  for (int i=0; i<DATA_LEN; i++)
  {
    if (serialarray[i] != parallelarray[i])
    {
      printf("Data check failed offset %d\n", i );
      return;
    }
  }
}


int main( int argc, char** argv )
{
  /* loop variables */
  int x,y;

  /* Create matrixes */
  unsigned int * input_array;
  unsigned int * serial_array;
  unsigned int * output_array;
  unsigned int * filter_list;

  /* Makde csv pointers */
  FILE *sdf;
  FILE *sff;
  FILE *pdf;
  FILE *pff;
  FILE *pdu;
  FILE *pdsch;

  /* Initialize csvs */
  sdf=fopen("serial-data.csv","a");
  sff=fopen("serial-filter.csv","a");
  pdf=fopen("parallel-data.csv","a");
  pff=fopen("parallel-filter.csv","a");
  pdu=fopen("parallel-data-unrolled.csv","a");
  pdsch=fopen("parallel-data-scheduled.csv","a");

  /* Initialize headers */
  struct stat st;

  stat("serial-data.csv", &st);
  if (st.st_size < 1)
    fprintf(sdf, "filter length,sec,us\n");

  stat("serial-filter.csv", &st);
  if (st.st_size < 1)
    fprintf(sff, "filter length,sec,us\n");

  stat("parallel-data.csv", &st);
  if (st.st_size < 1)
    fprintf(pdf, "threads,sec,us\n");

  stat("parallel-filter.csv", &st);
  if (st.st_size < 1)
    fprintf(pff, "threads,sec,us\n");

  stat("parallel-data-unrolled.csv", &st);
  if (st.st_size < 1)
    fprintf(pdu, "unroll,sec,us\n");

  stat("parallel-data-scheduled.csv", &st);
  if (st.st_size < 1)
    fprintf(pdsch, "schedule,sec,us\n");

  /* Initialize the data. Values don't matter much. */
  posix_memalign ( (void**)&input_array, 4096,  DATA_LEN * sizeof(unsigned int));
//  input_array = (unsigned int*) posix_memalign ( DATA_LEN * sizeof(unsigned int), 4096);
  for (x=0; x<DATA_LEN; x++)
  {
    input_array[x] = x % 2048;
  }

  /* start with an empty *all zeros* output array */
  posix_memalign ( (void**)&output_array, 4096, DATA_LEN * sizeof(unsigned int));
  memset ( output_array, 0, DATA_LEN );
  posix_memalign ( (void**)&serial_array, 4096, DATA_LEN * sizeof(unsigned int));
  memset ( serial_array, 0, DATA_LEN );

  /* Initialize the filter. Values don't matter much. */
  filter_list = (unsigned int*) malloc ( FILTER_LEN * sizeof(unsigned int));
  for (y=0; y<FILTER_LEN; y++)
  {
    filter_list[y] = y;
  }


  omp_set_num_threads(THREADS);

  serialDataFirst ( DATA_LEN, input_array, serial_array, 512, filter_list, sdf );
  memset ( output_array, 0, DATA_LEN );

  for (int i = 0; i < 20; i++)
  {
    //parallelDataFirstUnrolled ( DATA_LEN, input_array, output_array, 512, filter_list, pdu);
    //checkData ( serial_array, output_array );
    //memset ( output_array, 0, DATA_LEN );

    parallelDataFirstScheduledDynamic ( DATA_LEN, input_array, output_array, 512, filter_list, pdsch);
    checkData ( serial_array, output_array );
    memset ( output_array, 0, DATA_LEN );

    parallelDataFirstScheduledGuided ( DATA_LEN, input_array, output_array, 512, filter_list, pdsch);
    checkData ( serial_array, output_array );
    memset ( output_array, 0, DATA_LEN );

    parallelDataFirstScheduledStatic32 ( DATA_LEN, input_array, output_array, 512, filter_list, pdsch);
    checkData ( serial_array, output_array );
    memset ( output_array, 0, DATA_LEN );

    parallelDataFirstScheduledStatic64 ( DATA_LEN, input_array, output_array, 512, filter_list, pdsch);
    checkData ( serial_array, output_array );
    memset ( output_array, 0, DATA_LEN );
  }

  /* Execute at a variety of filter lengths */
  //for ( int filter_len =1; filter_len<=FILTER_LEN; filter_len*=2) 

  /*
  for (int threads=1; threads <= THREADS; threads*=2)
  {
  omp_set_num_threads(threads);
    for ( int filter_len =512; filter_len<=512; filter_len*=2) 
    {
      serialDataFirst ( DATA_LEN, input_array, serial_array, filter_len, filter_list, sdf );
      memset ( output_array, 0, DATA_LEN );

      parallelDataFirstUnrolled ( DATA_LEN, input_array, output_array, filter_len, filter_list, pdu);
      checkData ( serial_array, output_array );
      memset ( output_array, 0, DATA_LEN );

      //serialFilterFirst ( DATA_LEN, input_array, output_array, filter_len, filter_list, sff );
      //checkData ( serial_array, output_array );
      //memset ( output_array, 0, DATA_LEN );

      //parallelFilterFirst ( DATA_LEN, input_array, output_array, filter_len, filter_list, pff, threads);
      //checkData ( serial_array, output_array );
      //memset ( output_array, 0, DATA_LEN );

      //parallelDataFirst ( DATA_LEN, input_array, output_array, filter_len, filter_list, pdf, threads);
      //checkData ( serial_array, output_array );
      //memset ( output_array, 0, DATA_LEN );
    }
  }
  */

  fclose(sdf);
  fclose(sff);
  fclose(pdf);
  fclose(pff);
  fclose(pdu);
}

