#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#define STACK_SIZE 4096
ucontext_t c1, c2, c3;
void func1(void) { printf("In func1\n"); }
void func2(int arg) { printf("In func2, argument = %d\n", arg); }
int main()
{
	int argument = 442;
	getcontext(&c1);
	c1.uc_link = &c3;
	c1.uc_stack.ss_sp = malloc(STACK_SIZE);
	c1.uc_stack.ss_size = STACK_SIZE;
	makecontext(&c1, (void (*)(void))func1, 0);
	getcontext(&c2);
	c2.uc_link = &c3;
	c2.uc_stack.ss_sp = malloc(STACK_SIZE);
	c2.uc_stack.ss_size = STACK_SIZE;
	makecontext(&c2, (void (*)(void))func2, 1, argument);
	getcontext(&c3);
	printf("Switching to thread 1\n");
	swapcontext(&c3, &c1);
	printf("Switching to thread 2\n");
	swapcontext(&c3, &c2);
	printf("Exiting\n");
	free(c1.uc_stack.ss_sp);
	free(c2.uc_stack.ss_sp);
	return 0;
}