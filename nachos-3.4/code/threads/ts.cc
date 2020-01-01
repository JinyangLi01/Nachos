#include "copyright.h"
#include "system.h"

void ts() {
    printf("%10s%20s%10s\n", "Thread ID", "Name", "Status");
    for (int i = 0; i < 128; ++i) {
    	if (tidArray[i] != 0) {
    	    printf("%10d%20s", threadArray[i]->GetTid(), threadArray[i]->getName());
    	    switch(threadArray[i]->GetStatus()) {
    	    	case JUST_CREATED: printf("%10s","JUST_CREATED"); break;
    	    	case RUNNING: printf("%10s","RUNNING"); break;
    	    	case READY: printf("%10s","READY"); break;
    	    	case BLOCKED: printf("%10s","BLOCKED"); break;
    	    	default: break;
    	    }
    	    printf("\n");
    	}
    }
}

