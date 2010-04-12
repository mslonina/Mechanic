#include <stdio.h>
#include <stdlib.h>
#include <popt.h>

int main(int argc, char *argv[]) {
   int t = 0;   
   poptContext optCon; 
   struct poptOption optionsTable[] = {
        { "test", 't', POPT_ARG_INT, &t, 0,
        "example", "TEST" },
        POPT_AUTOHELP
        { NULL, 0, 0, NULL, 0 }
   };
   optCon = poptGetContext(NULL, argc, (const char**)argv, optionsTable, 0);
   poptFreeContext(optCon);
   exit(0);
}
