#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define N 196608 
#define M 512 

int main (int argc, char *argv[]) {
  int i, j, k;
  char A[N][M];
  int nop;
  int iter;
  unsigned long n=0;

  if (argc != 3) {
    printf("Error (# Input parameters = %d). Usage: ./mm-microbenchmarks nops iters\n", argc);
    return 1;
  }   

  nop = atoi(argv[1]);
  iter = atoi(argv[2]);

  nop = nop * N;

  if (iter < 1) {
    while (1) {
      for (j=0; j<N; j++) {
	A[j][0] ++;
	n++;
      }
      for (k=0; k<nop; k++) {
	asm("nop");
      }
    }
  }

  else {
    for (i=0; i<iter; i++) {
      for (j=0; j<N; j++) {
	A[j][0] ++;
	n++;
      }
      for (k=0; k<nop; k++) {
        asm("nop");
      }
    }
  }

  printf("The number of accesses should be %lu\n", n);

  return 0;
}
