
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>





/* print the data to the screen */
void print(int* data, int rank,int keys_for_each_process) {
  int i;
  printf(" Local sorted lists of Process %d: ",rank);
  for (i = 0; i < keys_for_each_process; i++) {
    printf("%d ", data[i]);
  }
  printf("\n");
}

/* comparison function for qsort */
int cmp(const void* ap, const void* bp) {
  int a = * ((const int*) ap);
  int b = * ((const int*) bp);

  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

/* find the index of the largest item in an array */
int max_index(int* data,int keys_for_each_process) {
  int i, max = data[0], maxi = 0;

  for (i = 1; i < keys_for_each_process; i++) {
    if (data[i] > max) {
      max = data[i];
      maxi = i;
    }
  }
  return maxi;
}

/* find the index of the smallest item in an array */
int min_index(int* data ,int keys_for_each_process) {
  int i, min = data[0], mini = 0;

  for (i = 1; i < keys_for_each_process; i++) {
    if (data[i] < min) {
      min = data[i];
      mini = i;
    }
  }
  return mini;
}


/* do the parallel odd/even sort */
void parallel_sort(int* data, int rank, int size,int keys_for_each_process) {
  int i;

  /* the array we use for reading from partner */
  int other[keys_for_each_process];

  /* we need to apply P phases where P is the number of processes */
  for (i = 0; i < size; i++) {
    /* sort our local array */
    qsort(data,keys_for_each_process, sizeof(int), &cmp);

    /* find our partner on this phase */
    int partener;

    /* if it's an even phase */
    if (i % 2 == 0) {
      /* if we are an even process */
      if (rank % 2 == 0) {
        partener = rank + 1;
      } else {
        partener = rank - 1;
      }
    } else {
      /* it's an odd phase - do the opposite */
      if (rank % 2 == 0) {
        partener = rank - 1;
      } else {
        partener = rank + 1;
      }
    }

    /* if the partener is invalid, we should simply move on to the next iteration */
    if (partener < 0 || partener >= size) {
      continue;
    }

    /* do the exchange - even processes send first and odd processes receive first
     * this avoids possible deadlock of two processes working together both sending */
    if (rank % 2 == 0) {
      MPI_Send(data, keys_for_each_process, MPI_INT, partener, 0, MPI_COMM_WORLD);
      MPI_Recv(other, keys_for_each_process, MPI_INT, partener, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
      MPI_Recv(other, keys_for_each_process, MPI_INT, partener, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(data, keys_for_each_process, MPI_INT, partener, 0, MPI_COMM_WORLD);
    }

    /* now we need to merge data and other based on if we want smaller or larger ones */
    if (rank < partener) {
      /* keep smaller keys */
      while (1) {
        /* find the smallest one in the other array */
        int mini = min_index(other,keys_for_each_process);

        /* find the largest one in out array */
        int maxi = max_index(data,keys_for_each_process);

        /* if the smallest one in the other array is less than the largest in ours, swap them */
        if (other[mini] < data[maxi]) {
          int temp = other[mini];
          other[mini] = data[maxi];
          data[maxi] = temp;
        } else {
          /* else stop because the smallest are now in data */
          break;
        }
      }
    } else {
      /* keep larger keys */
      while (1) {
        /* find the largest one in the other array */
        int maxi = max_index(other,keys_for_each_process);

        /* find the largest one in out array */
        int mini = min_index(data,keys_for_each_process);

        /* if the largest one in the other array is bigger than the smallest in ours, swap them */
        if (other[maxi] > data[mini]) {
          int temp = other[maxi];
          other[maxi] = data[mini];
          data[mini] = temp;
        } else {
          /* else stop because the largest are now in data */
          break;
        }
      }
    }
  }
}

int main(int argc, char** argv) {
  /* our rank and size */
  int rank, size;

  /* our processes data */
  int *keys=NULL;
  
  int keys_for_each_process;
  int num_of_keys;
  int *global_list;
  
  
  /*gernerate  the size of the recv array*/
		int dest = 0;
		/*for gatherv usage.*/
		int *rc,*disp;
  
  
  /* initialize MPI */
  MPI_Init(&argc, &argv);

  /* get the rank (process id) and size (number of processes) */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /*Process 0 should read in Num of keys.  */
  if(rank==0)
  {
	  printf("Enter the number of keys that you want to sort.\n");
	  scanf("%d",&num_of_keys);
	  
	  /*num_of_keys/size is the num of keys for each process.*/
	  keys_for_each_process=num_of_keys/size;
	  global_list=(int*)malloc(num_of_keys*sizeof(int));
	  
			  
		
		 
		/*gernerate  the size of the recv array*/
		
		int i;
		rc = (int*)malloc(size*sizeof(int));
		for(i=0;i<size;i++)
		{
			rc[i]=keys_for_each_process;
			
			
		}


		/*calculate displacements.*/
		disp = (int*)malloc(size*sizeof(int));
		for(i=0;i<size;i++)
		{
			disp[i]=i*keys_for_each_process;
			
			
		}
		 
		
	  
  }
  
  
   /*broadcast it to other process.*/
   MPI_Bcast(&keys_for_each_process,1,MPI_INT,0,MPI_COMM_WORLD);
  
 

 /* initialize the data to random values based on rank (so they're different) */
    keys=(int*)malloc(keys_for_each_process*sizeof(int));
	
	  /* Intializes random number generator */
	 
	  int i;
	  srand(rank);
	  for (i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
		keys[i] = rand( ) % 100;
	  }
		  
	  
	  
	  
  /* do the parallel odd/even sort */
  parallel_sort(keys, rank, size,keys_for_each_process);

  
  
					 
  /* now print our data */
  print(keys, rank ,keys_for_each_process);
  
 
  

  /*Merge the global list */
  MPI_Gatherv ( keys, keys_for_each_process, MPI_INTEGER, global_list, rc, disp, MPI_INTEGER , dest, MPI_COMM_WORLD);
 
					     
   if(rank==0)
   {
	   printf("We generate %d random number and use parallel Odd-Even Transposition Sort to get the sorting result.\n",num_of_keys);
	   for(i=0;i<num_of_keys ;i++)
	   {
		   printf("%d ",global_list[i]);
	   }
	   
	   
	   
   }
  /* quit MPI */
  MPI_Finalize( );
  return 0;
}