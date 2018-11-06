#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <random>

//#define HEIGHT 40
//#define WIDTH 70
const int GRID_SIZE_X = 70; //gridの縦と横
const int GRID_SIZE_Y = 40;
#define BUFSIZE 1000
const bool IS_INPUT = true;

bool cell[GRID_SIZE_X][GRID_SIZE_Y];

void init_cells(){
  int x = 0, y;
  char buf[BUFSIZE];
  FILE *fp;
  const char *filename = "tmp.txt";

  if (IS_INPUT){
    if ((fp = fopen(filename, "r")) == NULL) {
      printf("error: can't open %s\n", filename);
      return;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
      size_t len = strlen(buf) - 1;
      for (x = 0; x < len; x++){
        if (buf[x] == ' '){
          cell[x][y] = false;
        } else {
          cell[x][y] = true;
        }
      }
      y++;
    }
    fclose(fp);
  } else {
    // 乱数に従って最初の分布を生成
    std::default_random_engine generator;
    std::bernoulli_distribution distribution(0.5);
    std::uniform_int_distribution<> dist1(-1.0, 1.0);
    for (int y = 0; y < GRID_SIZE_Y; y++){
        for (int x = 0; x < GRID_SIZE_X; x++){
            if (distribution(generator)){
                cell[x][y] = true;
            } else {
                cell[x][y] = false;
            }
        }
    }
  }
}


void print_cells(FILE *fp)
{
  int x, y;

  fprintf(fp, "----------\n");

  for (y = 0; y < GRID_SIZE_Y; y++) {
    for (x = 0; x < GRID_SIZE_X; x++) {
      const char c = (cell[x][y] == true) ? '#' : ' ';
      fputc(c, fp);
    }
    fputc('\n', fp);
  }

  fflush(fp);

  sleep(1);
}



int count_adjacent_cells(int x, int y){
  int n = 0;
  int dx, dy;
  for (dx = x - 1; dx <= x + 1; dx++) {
    if (dx < 0 || dx >= GRID_SIZE_X) continue;
    for (dy = y - 1; dy <= y + 1; dy++) {
      if (dx == x && dy == y) continue;
      if (dy < 0 || dy >= GRID_SIZE_Y) continue;
      n += (int)cell[dx][dy];
    }
  }
  return n;
}

void update_cells(){
  int x, y;
  bool cell_next[GRID_SIZE_X][GRID_SIZE_Y];

  for (x = 0; x < GRID_SIZE_X; x++) {
    for (y = 0; y < GRID_SIZE_Y; y++) {
      cell_next[x][y] = false;
      const int n = count_adjacent_cells(x, y);
      if (n == 2){
        cell_next[x][y] = cell[x][y];
      } else if (n == 3){
        cell_next[x][y] = true;     
      } else {                  // n == 1,4 death
        cell_next[x][y] = false;
      }
    }
  }

  for (x = 0; x < GRID_SIZE_X; x++) {
    for (y = 0; y < GRID_SIZE_Y; y++) {
      cell[x][y] = cell_next[x][y];
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