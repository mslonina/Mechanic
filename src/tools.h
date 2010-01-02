#ifndef MPIFARM_TOOLS_H
#define MPIFARM_TOOLS_H

typedef struct{
  float aa;
  float bb;
} yourData;

yourData *makeYourData(void){
 yourData *pointer = malloc(sizeof(yourData));
 if(pointer == NULL)
   return NULL;

 pointer->aa = 0.0;
 pointer->bb = 0.0;

 return pointer;
  
};

#endif
