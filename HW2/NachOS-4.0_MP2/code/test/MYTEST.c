#include "syscall.h"

//  ../build.linux/nachos -e MYTEST

int glob = 123;
int main()
{
	int n;
	const int a = 1;
	for (n = 9; n > 5; n--)
	{
		PrintInt(n);
	}
	
	PrintInt(a);
	a = 4;
	PrintInt(a);

	return 0;
	//Halt();
}
