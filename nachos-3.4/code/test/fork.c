/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int fd1, fd2;
char temp[50];
int len;

void test() {
    Create("TestFile_1");
    fd1 = Open("TestFile");
    fd2 = Open("TestFile_1");  
    len = Read(temp, 50, fd1);
    Write(temp, len, fd2);
    Close(fd1);
    Close(fd2);
}

int
main()
{
    Fork((void *)test);
    //Halt();
    /* not reached */
}
