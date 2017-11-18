#include <stdio.h>

int main(int argc, char **argv)
{
  int x = 1;
  printf("Hello x = %d\n", x);
 
asm("incl %0\n\t"
    :"=r"(x)
    :"r"(x)); 
  //
  // Put in-line assembly here to increment
  // the value of x by 1 using in-line assembly
  //

  printf("Hello x = %d after increment\n", x);

  if(x == 2){
    printf("OK\n");
  }
  else{
    printf("ERROR\n");
  }
}

