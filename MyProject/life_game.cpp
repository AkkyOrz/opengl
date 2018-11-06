#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define HEIGHT 40
#define WIDTH 70

int cell[HEIGHT][WIDTH];

int sleep_milsec(unsigned long x){
  clock_t c1 = clock(), c2;

  do { 
    if ((c2 = clock()) == (clock_t)-1)
      return 0;
  } while (1000.0 * (c2 - c1) / CLOCKS_PER_SEC < x);
  return 1;
}

void init_cells()
{
  int i, j;
  srand(time(NULL));

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      if (rand() % 10){ 
        cell[i][j] = 0;
      } else {
        cell[i][j] = 1;
      }
    }
  }
}

double count_propotion(){
  int sum = 0;
  double propotion;
  int i,j;
  for (i = 0; i < HEIGHT; i++){
    for (j = 0; j < WIDTH; j++){
      sum += cell[i][j];
    }
  }
  propotion = (double)sum /(HEIGHT * WIDTH);

  return propotion;
}

void print_cells(FILE *fp)
{
  int i, j;

  fprintf(fp, "----------\n");

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      const char c = (cell[i][j] == 1) ? '#' : ' ';
      fputc(c, fp);
    }
    fputc('\n', fp);
  }

  fprintf(fp, "propotion = %f\n", count_propotion());
  fflush(fp);



  sleep_milsec(500);                                        ///高速化
}


int cycle_height(int i){                                           //境界を消去
  if (i >= HEIGHT){
    return i - HEIGHT;
  } else if (i < 0){
    return i + HEIGHT;
  } else{
    return i;
  }
}

int cycle_width(int j){                                          //境界を消去
  if (j >= WIDTH){
    return j - WIDTH;
  } else if (j < 0){
    return j + WIDTH;
  } else {
    return j;
  }
}

int count_adjacent_cells(int i, int j)
{
  int n = 0;
  int k, l;
  for (k = i - 1; k <= i + 1; k++) {
    for (l = j - 1; l <= j + 1; l++) {
      if (k == i && l == j) continue;
      n += cell[cycle_height(k)][cycle_width(l)];             //境界を消去
    }
  }

  return n;
}

void update_cells()
{
  int i, j;
  int cell_next[HEIGHT][WIDTH];

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      cell_next[i][j] = 0;
      const int n = count_adjacent_cells(i, j);

      if (n == 2){                            //n == 2 continue
        cell_next[i][j] = cell[i][j];
      } else if (n == 3){ 
        cell_next[i][j] = 1;                    //n == 3 birth
      } else {                  // n == 1,4,5,6,7,8 death
        cell_next[i][j] = 0;
      }

      if (pow(i - 10, 2) + pow(j - 20, 2) <= 100){                     //(縦に10,横に20)行った場所を中心に生存環境を易しくする
        if (n == 1 && n == 4){                                  //n == 1,4 continue
          cell_next[i][j] = cell[i][j];                         
        } else if (n == 2 && n == 3){                           //n == 2,3 birth
          cell_next[i][j] = 1;
        } else {                                                //n else death 
          cell_next[i][j] = 0;
        }
      }

      if (pow(i - 30, 2) + pow(j - 50, 2) <= 64){                     //(30,50)の場所の環境を過酷にする
        if (n == 2 && n == 3){                                  //n == 2,3 continue
          cell_next[i][j] = cell[i][j]; 
        } else {                                                //n else death 
          cell_next[i][j] = 0;
        }                            
      }
    }
  }

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      cell[i][j] = cell_next[i][j];
    }
  }

  while (count_propotion() < 0.05){                         //強制増殖
    if (cell[i = rand()%HEIGHT][j = rand()%WIDTH] == 0){
      cell[i][j] = 1;
    }
  }
  while (count_propotion() > 0.2) {                         //強制死滅
    if (cell[i = rand()%HEIGHT][j = rand()%WIDTH] == 0){
      cell[i][j] = 0;
    }
  }

}


int main()
{
  int gen;
  FILE *fp;

  if ((fp = fopen("cells.txt", "a")) == NULL) {
    fprintf(stderr, "error: cannot open a file.\n");
    return 1;
  }

  init_cells();
  print_cells(fp);

  for (gen = 1;; gen++) {
    printf("generation = %d\n", gen);
    update_cells();
    print_cells(fp);
  }

  fclose(fp);

  return 0;
}