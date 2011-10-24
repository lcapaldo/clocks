#include "clock.h"
#include <stdio.h>

int main()
{

  HANDLE clk = CreateClock(1000);
  int i = 0;
  while(i < 10)
  {
    WaitForSingleObject(clk, INFINITE);
    printf("tick\n");
    ++i;
  }
  StopClock(clk);
}
