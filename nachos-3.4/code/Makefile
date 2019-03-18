# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

MAKE = make  #以后的gmake都可以用$(MAKE)代替
LPR = lpr

#all: 是要执行的所有make，我们可以看到执行该make之后其实是对每个代码包里面的.cc或者.h文件里面都做makefile，其中相应的makefile写在相应的代码包中。以Threads为例，code/threads包下面也有makefile，那个makefile才执行了编译code/threads包下面.cc(.h)的操作。在以后的编程中，比如我们只修改了threads代码包下面的.cc或者.h文件，我们就可以直接cd到threads/下面执行make，而不用把整个工程编译。

#为什么要写$(MAKE) nachos和$(MAKE) depend两次呢，这是因为在Makefile.common文件中，既有nachos：(其中$(PROGRAM)的值就是nachos) 又有depend：，一次make只能生成一个，如果只是make，那么会自动寻找第一个，所以要写两次，在depend：里面，只要是为了应对不同的机型采取的编译。

#makefile.dep是为了说明不同的机型所采用的，我所用的处理器就是i386的，所以Host=-DHOST_i386是没错的，如果你的机型是Sun APARC 或者HP，那么把相应的注释除去就好。

#其中定义了一些其他makefile文件可能会用到的常量，比如$(CC)就是g++，这个语句在以后还会遇见，提前注意一下。


all: 
	cd threads; $(MAKE) depend
	cd threads; $(MAKE) nachos
	cd userprog; $(MAKE) depend 
	cd userprog; $(MAKE) nachos 
	cd vm; $(MAKE) depend
	cd vm; $(MAKE) nachos 
	cd filesys; $(MAKE) depend
	cd filesys; $(MAKE) nachos 
	cd network; $(MAKE) depend
	cd network; $(MAKE) nachos 
	cd bin; make all
	cd test; make all

# don't delete executables in "test" in case there is no cross-compiler
clean:
	/bin/csh -c "rm -f *~ */{core,nachos,DISK,*.o,swtch.s,*~} test/{*.coff} bin/{coff2flat,coff2noff,disassemble,out}"

print:
	/bin/csh -c "$(LPR) Makefile* */Makefile"
	/bin/csh -c "$(LPR) threads/*.h threads/*.cc threads/*.s"
	/bin/csh -c "$(LPR) userprog/*.h userprog/*.cc" 
	/bin/csh -c "$(LPR) filesys/*.h filesys/*.cc
	/bin/csh -c "$(LPR) network/*.h network/*.cc 
	/bin/csh -c "$(LPR) machine/*.h machine/*.cc
	/bin/csh -c "$(LPR) bin/noff.h bin/coff.h bin/coff2noff.c"
	/bin/csh -c "$(LPR) test/*.h test/*.c test/*.s"
