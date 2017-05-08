#include "ucontext.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

// TODO context array is changed to ThreadInfo array
// 		Parallel threads will be created 
// 		Signals and alarm will be resarched in detail
 

#define STACK_SIZE 4096
ucontext_t* context_arr;
int current_context = 0;
int context_num = 0;
struct ThreadInfo
{
	ucontext_t context;
	int state;
};
volatile sig_atomic_t print_flag = false;

void func(int n,int tabnum)
{
	//printf("%d\n",n);

	int x = 0;
	int y = 0;
	for (x = 1 ; x <= n ; x++){
		for (y = 1 ; y < tabnum ; y++)
			printf("\t");
		printf("%d\n", x);
		usleep(100000);
	}
	free(context_arr[current_context].uc_stack.ss_sp); 
	context_arr[current_context].uc_stack.ss_size = 0;
	alarm(0);
}
void context_swapper(int sig)
{
	print_flag = true;

	do
	{
		current_context++;		
		current_context = current_context % context_num;
	}while((context_arr[current_context].uc_stack.ss_size)==0);

	if(context_arr[current_context].uc_stack.ss_size)
		swapcontext(&context_arr[0], &context_arr[current_context]);
}

int i ; 
int main(int argc, char const *argv[])
{
	ucontext_t c;

	context_arr = malloc(argc*sizeof(ucontext_t));
	for (i = 1; i < argc ; i++)
	{
		getcontext(&c);
		//printf("%d\n",atoi(argv[i]));
		c.uc_link = &context_arr[0];
		c.uc_stack.ss_sp = malloc(STACK_SIZE);
		c.uc_stack.ss_size = STACK_SIZE;
		makecontext(&c,(void (*)(void))func,2,atoi(argv[i]),i);
		context_arr[i] = c;
		//free(c.uc_stack.ss_sp);
	}
	context_num = argc;
	signal( SIGALRM, context_swapper ); // Install handler first,
	alarm( 1 ); // before scheduling it to be called.
	while(1)
	{
		if(print_flag)
		{
	    	alarm( 1 ); // before scheduling it to be called.
	    	print_flag = false;
	    }
	}
	//sleep(10000);
	free(context_arr);
	free(c.uc_stack.ss_sp);
	/* code */
	return 0;
}