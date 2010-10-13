#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main(){

  void *handler;
  double (*cosine) (double);
  char *error;

  handler = dlopen("libm.dylib", RTLD_LAZY);
  if(!handler){
    printf("libm not found\n");
    exit(1);
  }

  cosine = dlsym(handler, "cos");
  if ((error = dlerror()) != NULL) exit(1);

  /*printf("Cos %f\n", cosine(2.0));*/
  dlclose(handler);
  return 0;
}
