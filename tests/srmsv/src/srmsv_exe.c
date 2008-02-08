#if (NCS_SRMA_EXE == 1 )
#include<stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  char *mem[1] ;
  int i=0;
/*  printf("ARGV : %s\n",argv[1]);*/
  if(!(strcmp(argv[1],"MEM")))
  {
     for(i=0; i<10000000;i++)
     {
       mem[0] =(char *) malloc(sizeof(char *)); 
       if(mem[0] == NULL)
          sleep(200);
     }
  }
  if(!(strcmp(argv[1],"CPU_UTIL")))
  {
     while(1){};
  }
   
  sleep(200);
  return 1;
}
#endif
