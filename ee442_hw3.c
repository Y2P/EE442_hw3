#include "ucontext.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

// Predefined Thread States
#define FINISHED 0
#define READY 1 
#define RUNNING 2
#define MAX_THREAD 5

#define STACK_SIZE 4096

// Global variables are  defined here
int current_context = 0; // For current thread index	
int context_num = 0;
int prev_context= 0;	// Holding previous thread index

// Structure for thread information ( Context and state)
struct ThreadInfo
{
	ucontext_t context;
	int state; 
};

// Thread Array used for the round robin
struct ThreadInfo ThreadArr[MAX_THREAD];

// ISR for catching CTRL-C
void  INThandler(int sig)
{
	int i ;

	printf("\nReleasing the allocated memory\n");
	// Deleting the dynamically allocated memory
	for(i = 1; i < MAX_THREAD ; i++)
	{
		free(ThreadArr[i].context.uc_stack.ss_sp);
	}

	// 	And exit to terminal
	printf("Exiting... (Bye bye) \n");

	exit(0);

}


// Method for checking FINISHED places in given Thread Array
// Returns the first FINISHED place. If there is not, returns 0
int HasEmptyPlace(struct ThreadInfo ThreadArr[MAX_THREAD])
{
	int i;
	for (i = 1; i < MAX_THREAD ; i++)
	{
		if(ThreadArr[i].state == FINISHED)
			return i;
	}
	return 0;
}
// Worker function for the threads. Takes two parameters (n, tabnum)
// n is for the number of count
// tabnum is for number of printed tabs.
void func(int n,int tabnum)
{
	int x = 0;
	int y = 0;
	ThreadArr[current_context].state = RUNNING;

	// Print operation is done here
	for (x = 0 ; x < n ; x++){
		for (y = 0 ; y < tabnum ; y++)
			printf("\t");
		printf("%d\n", x);
		usleep(100000);
	}
	// Free the place in the thread array for the waiting threads in the queue
	ThreadArr[current_context].context.uc_stack.ss_size = 0;
	ThreadArr[current_context].state = FINISHED;

	// Switch to the new thread or main
	raise(SIGALRM);
}

// Interrupt handler function for the scheduler
// This interrupt occurs in 1 second after its alarm is set
// At the end of 1 second, this function operates and switch the context
// to a thread in the thread array which Round Robin scheduling proposes
void context_swapper(int sig)
{
	// For recording the previous context, take the index of it.
	prev_context = current_context;

	// Update the current context
	do
	{
		current_context++;			
		current_context = current_context % MAX_THREAD;
	}while(ThreadArr[current_context].state != READY && ThreadArr[current_context].state != RUNNING );

	// Set the alarm to 1 second again
	alarm( 1 ); // before scheduling it to be called.

	// Save the previous context
	getcontext(&ThreadArr[prev_context].context);
	// Switch to new context
	swapcontext(&ThreadArr[prev_context].context, &ThreadArr[current_context].context);
}

int main(int argc, char const *argv[])
{
	// To end program properly, CTRL-C is caught by signal interrupt
    printf("CTRL-C to quit\n");
	signal(SIGINT, INThandler);

	int i ;
	int idx;

	// Context switching interrupt is defined here
	signal( SIGALRM, context_swapper ); 
	alarm( 1 ); 
	ucontext_t c;

	// Initialize the threads 
	for (i = 0; i < MAX_THREAD ; i++)
	{
		ThreadArr[i].state = FINISHED;
	}

	// First thread is the main function
	getcontext(&c);
	ThreadArr[0].context = c;
	ThreadArr[0].state = READY;

	// For each thread 
	for (i = 1; i < argc ; i++)
	{
		// Wait until an empty space exists in thread array
		while((idx = HasEmptyPlace(ThreadArr)) == 0)
			raise(SIGALRM); // Eliminate the idle state
		
		getcontext(&c);

		// Initialize and create a new thread with its context
		c.uc_link = &ThreadArr[0].context;
		c.uc_stack.ss_sp = malloc(STACK_SIZE);
		c.uc_stack.ss_size = STACK_SIZE;

		makecontext(&c,(void (*)(void))func,2,atoi(argv[i]),i);
	
		// Place it to the next empty place and turn its space to READY
		ThreadArr[idx].context = c;
		ThreadArr[idx].state = READY;
	}
	while(1)
		raise(SIGALRM); // Eliminate the idle state
	
	return 0;
}