#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

#define VECSIZE 65000
#define ITERATIONS 20
double When()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}
main(int argc, char *argv[]) 
{
	int iproc, nproc,i, iter, proc;
	char host[255], message[55];
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &iproc);

	gethostname(host,253);
//	printf("I am proc %d of %d running on %s\n", iproc, nproc,host);
        // each process has an array of VECSIZE double: ain[VECSIZE]
        double ain[VECSIZE], aout[VECSIZE];
        int  ind[VECSIZE];
        struct {
            double val;
            int   rank;
        } in[VECSIZE], out[VECSIZE],temp[VECSIZE];
        int myrank, root = 0;

        MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
        // Start time here
        srand(myrank+5);
        double start = When();
        for(iter = 0; iter < ITERATIONS; iter++) {
	  for(i = 0; i < VECSIZE; i++) {
	    ain[i] = rand();
  //          printf("init proc %d [%d]=%f\n",myrank,i,ain[i]);
	  }
	  for (i=0; i<VECSIZE; ++i) {
	    in[i].val = ain[i];
	    in[i].rank = myrank;
	  }
	  
//	  printf("%d: Before reduce\n",iproc);
//	  for(i=0;i<VECSIZE;i++) {
//		  printf("%d. val: %f rank: %d\n", i, in[i].val, in[i].rank);
//	  }
	  
//	  MPI_Reduce( in, out, VECSIZE, MPI_DOUBLE_INT, MPI_MAXLOC, root, MPI_COMM_WORLD);
	  if(myrank == root) {
		  for(proc=0;proc<nproc;proc++) {
			  if(proc==0) {
				  for(i=0;i<VECSIZE;i++) {
					  out[i].val = in[i].val;
					  out[i].rank = in[i].rank;
				  }
			  }
			  if(proc != 0) {
				  MPI_Recv(&temp,VECSIZE,MPI_DOUBLE_INT,proc,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				  for(i=0;i<VECSIZE;i++) {
					  if(temp[i].val > out[i].val) {
						  out[i].val = temp[i].val;
						  out[i].rank = temp[i].rank;
					  }
				  }
			  }
		  }
	  }
	  
//	  if(iproc == root) {
//		  printf("After reduce\n");
//		  for(i=0;i<VECSIZE;i++) {
//			  printf("%d. val: %f rank: %d\n", i, out[i].val, out[i].rank);
//		  }
//	  }
	  
	  else {
		  MPI_Send(&in,VECSIZE,MPI_DOUBLE_INT,root,0,MPI_COMM_WORLD);
	  }
	  
	  // At this point, the answer resides on process root
	  if (myrank == root) {
	      /* read ranks out
	       */
	      for (i=0; i<VECSIZE; ++i) {
  //                printf("root out[%d] = %f from %d\n",i,out[i].val,out[i].rank);
		  aout[i] = out[i].val;
		  ind[i] = out[i].rank;
	      }
	  }
	  // Now broadcast this max vector to everyone else.
//	  MPI_Bcast(out, VECSIZE, MPI_DOUBLE_INT, root, MPI_COMM_WORLD);
	  if(myrank == root) {
		  for(i=1;i<nproc;i++) {
			  MPI_Send(&out,VECSIZE,MPI_DOUBLE_INT,i,0,MPI_COMM_WORLD);
		  }
	  }
	  else {
		  MPI_Recv(&out,VECSIZE,MPI_DOUBLE_INT,root,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	  }
	  for(i = 0; i < VECSIZE; i++) {
 //           printf("final proc %d [%d]=%f from %d\n",myrank,i,out[i].val,out[i].rank);
	  }
        }
	MPI_Finalize();
        double end = When();
        if(myrank == root) {
	  printf("Time %f\n",(end-start)/ITERATIONS);
        }
}
