#include <stdio.h>
int x;

int main (){

  x = 10;
  int y;

//  y = x && x/2;

  if (x && x/5){
    printf("5\n");
  }
  else {
    printf("else\n");
  }
  return 0;
}
