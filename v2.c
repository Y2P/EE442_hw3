#include "ucontext.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#define EMPTY 0
#define READY 1 
#define FINISHED  2
#define RUNNING 3
#define MAX_THREAD 5
// TODO context array is changed to ThreadInfo array
// 		Parallel threads will be created 
// 		Signals and alarm will be resarched in detail
 

#define STACK_SIZE 4096

int swap_flag = 0;
int current_context = 0;
int context_num = 0;
int prev_context= 0;
struct ThreadInfo
{
	ucontext_t context;
	int state; 
};


volatile sig_atomic_t print_flag = false;
struct ThreadInfo ThreadArr[MAX_THREAD];

// ISR for catching CTRL-C
void  INThandler(int sig)
{
	int i ;
	for(i = 0; i < MAX_THREAD ; i++)
	{
		free(ThreadArr[i].context.uc_stack.ss_sp);
	}

	exit(0);

}


int HasEmptyPlace(struct ThreadInfo ThreadArr[MAX_THREAD])
{
	int i;
	for (i = 1; i < MAX_THREAD ; i++)
	{
		if(ThreadArr[i].state == EMPTY)
			return i;
	}
	return 0;
}
void func(int n,int tabnum)
{
	int idx; 
	idx = (tabnum)%MAX_THREAD;
	if(idx == 0)
		idx++;
	//ThreadArr[idx].state = RUNNING;
	//printf("n: %d, tabnum: %d\n",n,tabnum);
	int x = 0;
	int y = 0;
	for (x = 0 ; x < n ; x++){
		for (y = 0 ; y < tabnum ; y++)
			printf("\t");
		printf("%d\n", x);
		usleep(100000);
	}

	//printf("%d\n",idx );
	//free(ThreadArr[current_context].context.uc_stack.ss_sp); 
	ThreadArr[idx].context.uc_stack.ss_size = 0;
	ThreadArr[idx].state = EMPTY;
	//current_context = 0;
	raise(SIGALRM);
	//alarm(1);
	//usleep(100);
	//printf("FINISHED: %d\n", n);

}
int idx;

void context_swapper(int sig)
{
	//printf("Signal hit\n");
	swap_flag = 1;
	int i ; 
	//printf("%d\n", current_context);
/*	for (i = 1; i < MAX_THREAD; i++)
	{
		//printf("%d\n",  ThreadArr[i].state);
		if(i == MAX_THREAD-1 && ThreadArr[i].state == EMPTY)
		{
	//		printf("EMPTY\n");
			alarm( 1 ); // before scheduling it to be called.
			return;
		}
		else if(ThreadArr[i].state != EMPTY)
			break;
	}
*/	
/*	prev_context = current_context;
	printf("PREV_context: %d state: %d\n",current_context,ThreadArr[current_context].state);

	do
	{
		//printf("%d/n",(ThreadArr[current_context].context.uc_stack.ss_size));
		current_context++;			
		printf("current_context: %d state: %d\n",current_context,ThreadArr[current_context].state);

		current_context = current_context % MAX_THREAD;
		//if(current_context == 0)
		//	current_context++;
	//	printf("Update current context: %d %d\n",current_context,prev_context);
	//	printf("%d\n", ThreadArr[current_context].state );
	}while(ThreadArr[current_context].state != READY);
	printf("Previous Context = %d Current Context = %d\n",prev_context,current_context);
	//while((ThreadArr[current_context].context.uc_stack.ss_size)==0);
	//printf("Signal hit done\n");
	//if(ThreadArr[current_context].context.uc_stack.ss_size)
	alarm( 1 ); // before scheduling it to be called.

*/
	getcontext(&ThreadArr[idx].context);
	swapcontext(&ThreadArr[idx].context, &ThreadArr[0].context);

	alarm(1);
}
//int idx[4] = {1 , 2 , 3, 4};
int i ;

int main(int argc, char const *argv[])
{
  //  printf("CTRL-C to quit\n");
	//signal(SIGINT, INThandler);

	signal( SIGALRM, context_swapper ); // Install handler first,
	alarm( 1 ); // before scheduling it to be called.
	ucontext_t c;

	for (i = 0; i < MAX_THREAD ; i++)
	{
		ThreadArr[i].state = EMPTY;
	}

	context_num = argc;
	getcontext(&c);
	ThreadArr[0].context = c;
	ThreadArr[0].state = READY;

	//ThreadArr = malloc(argc*sizeof(struct ThreadInfo));
	for (i = 1; i < argc ; i++)
	{
		prev_context = idx;

		while((idx = HasEmptyPlace(ThreadArr)) == 0);		
		getcontext(&c);
		
		c.uc_link = &ThreadArr[0].context;
		c.uc_stack.ss_sp = malloc(STACK_SIZE);
		c.uc_stack.ss_size = STACK_SIZE;

		makecontext(&c,(void (*)(void))func,2,atoi(argv[i]),i);
		//printf("%d\n",current_context );
		//printf("Main Loop\n");
		//printf("idx:%d\n", idx[(i-1)%4]);
		ThreadArr[idx].context = c;
		ThreadArr[idx].state = READY;
		printf("idx: %d\n",idx );

		while(swap_flag == 0);
		swap_flag = 0;
		getcontext(&ThreadArr[prev_context].context);
		swapcontext(&ThreadArr[prev_context].context, &ThreadArr[idx].context);

		//	printf("%d : %d\n",i%MAX_THREAD,ThreadArr[i%MAX_THREAD].state);
		//
		//	printf("%d\n",i );
	//	free(c.uc_stack.ss_sp);
	}

	while(1);

	//free(ThreadArr);
	/* code */
	return 0;
}