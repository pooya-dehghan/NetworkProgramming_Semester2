#include<time.h>
#include<stdio.h>

int main() {
  time_t timer;
  time(&timer);
  printf("Local time is :%s",ctime(&timer));
  return 0;
}