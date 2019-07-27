
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <dos.h>

#define SIZE 32

int Seed;

int LifeNow(int X, int Y)
  {
    return(peekb(0xA000, 320*Y+X));
  }

GetNextStage(int CX, int CY)
  {
  int X, Y;
  char LifeArray[SIZE][SIZE];

    for (X=CX+1; X<=CX+SIZE-1; X++)
      for (Y=CY+1; Y<=CY+SIZE-1; Y++)
        {
          if ( !!LifeNow(X-1,Y) ^
               !!LifeNow(X+1,Y) ^
               !!LifeNow(X,Y+1) ^
               !!LifeNow(X,Y-1) )
            LifeArray[X-CX][Y-CY] = 1;
          else
            LifeArray[X-CX][Y-CY] = 0;
        }
    for (X=CX+1; X<=CX+SIZE-1; X++)
      for (Y=CY+1; Y<=CY+SIZE-1; Y++)
        {
          if (LifeArray[X-CX][Y-CY] && (LifeNow(X,Y) < 3))
            pokeb(0xA000, 320*Y+X, LifeNow(X,Y)+1);
          else if (!LifeArray[X-CX][Y-CY])
            pokeb(0xA000, 320*Y+X, 0);
        }
  }

int Randomizer(int Seed, int Num)
  {
  int R1 = floor(52*log(Seed+Num));
    return(R1 % 4);
  }

StartingConfig(int CX, int CY)
  {
  int X, Y;
    Seed++;
    for (X = CX+1; X<=CX+SIZE-1; X++)
      for (Y = CY+1; Y<=CY+SIZE-1; Y++)
        pokeb(0xA000, 320*Y+X, Randomizer(Seed,(X-CX)+(Y-CY)*16));
  }

SetColor(c,r,g,b)
int c,r,g,b;
  {
    outportb(0x03C8, c);
    outportb(0x03C9, r);
    outportb(0x03C9, g);
    outportb(0x03C9, b);
  }


int X, Y;
char LCV;
int XS, YS;

main(int argc, char **argv)
  {
    srand((unsigned int) argv[1]);
    Seed = random(200)+1000;
    _AX=19;
    __int__(16);
    SetColor(3,56,16,0);
    SetColor(2,44,8,0);
    SetColor(1,32,0,0);
    XS = 320 / (SIZE) - 1;
    YS = 200 / (SIZE) - 1;
    for (X=0; X<=XS; X++)
      for (Y=0; Y<=YS; Y++)
        StartingConfig(X*(SIZE), Y*(SIZE));
    for (Y=0; Y<=YS; Y++)
      for (X=0; X<=XS; X++)
        for (LCV=1; LCV<SIZE; LCV++)
          {
            GetNextStage(X*(SIZE),Y*(SIZE));
          }
    while (!kbhit());
    _AX=3;
    __int__(16);
  }

