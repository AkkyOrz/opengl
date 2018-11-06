#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <random>
#include <string.h>

#define WINDOW_X (1000)
#define WINDOW_Y (1000)
#define WINDOW_NAME "test2"

// GL関係のプロトタイプ宣言
void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle();
void timer(int value);

class Point {
public:
    double x;
    double y;
    double z;
    Point()
    {
        x = y = z = 0.0;
    }
    Point(double x0, double y0, double z0)
    {
        x = x0;
        y = y0;
        z = z0;
    }
};

// 定数
GLdouble lightblue[] = {0.5, 1.0, 1.0}; // color
GLdouble red[] = {1.0, 0.5, 0.5}; // color
const int GRID_SIZE_X = 40; //gridの縦と横
const int GRID_SIZE_Y = 40;
const int BUFSIZE = 1000;
const double GRID_OFFSET = 0.5;
const bool IS_INPUT = false;  //true なら　tmp.txtから持ってくる false はランダム
const int TIME_SLICE = 300;

// lifegame関連のプロトタイプ宣言
int cycle_x(int x);
int cycle_y(int y);
void init_cells();
int count_adjacent_cells(int x, int y);
void update_cells();

// 物体描画関連のプロトタイプ宣言
void draw_pyramid();
void draw_cube(Point p, GLdouble cube_color[]);
void draw_grid();
void draw_lifegame();


// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 3 * GRID_SIZE_X;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
bool cell[GRID_SIZE_X][GRID_SIZE_Y];



int main(int argc, char *argv[]){
  /* OpenGLの初期化 */
  init_GL(argc,argv);

  /* このプログラム特有の初期化 */
  init();

  /* コールバック関数の登録 */
  set_callback_functions();

  /* メインループ */
  glutMainLoop();

  return 0;
}

void init_GL(int argc, char *argv[]){
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(WINDOW_X,WINDOW_Y);
  glutCreateWindow(WINDOW_NAME);
}

void init(){
  glClearColor(0.0, 0.0, 0.0, 0.0);         // 背景の塗りつぶし色を指定
  init_cells();
}

void set_callback_functions(){
  glutDisplayFunc(glut_display);
  glutKeyboardFunc(glut_keyboard);
  glutMouseFunc(glut_mouse);
  glutMotionFunc(glut_motion);
  glutPassiveMotionFunc(glut_motion);
  //glutIdleFunc(glut_idle);
  glutTimerFunc(TIME_SLICE , timer , 0);
}

void glut_keyboard(unsigned char key, int x, int y){
  switch(key){
  case 'q':
  case 'Q':
  case '\033':
    exit(0);
  }

  glutPostRedisplay();
}

void glut_mouse(int button, int state, int x, int y){
  if(button == GLUT_LEFT_BUTTON){
    if(state == GLUT_UP){
      g_isLeftButtonOn = false;
    }else if(state == GLUT_DOWN){
      g_isLeftButtonOn = true;
    }
  }

  if(button == GLUT_RIGHT_BUTTON){
    if(state == GLUT_UP){
      g_isRightButtonOn = false;
    }else if(state == GLUT_DOWN){
      g_isRightButtonOn = true;
    }
  }
}

void glut_motion(int x, int y){
  static int px = -1, py = -1;
  if(g_isLeftButtonOn == true){
    if(px >= 0 && py >= 0){
      g_angle1 += (double) -(x - px) / 20;
      g_angle2 += (double) -(y - py) / 20;
    }
    px = x;
    py = y;
  }else if(g_isRightButtonOn == true){
    if(px >= 0 && py >= 0){
      g_distance += (double) (y - py) / 20;
    }
    px = x;
    py = y;
  }else{
    px = -1;
    py = -1;
  }
  glutPostRedisplay();
}

void glut_display(){
  // まず投影変換
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, 1.0, 0.1, 200);  // ここでカメラの写せる範囲を指定している

  // つぎにモデル・ビュー変換
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  if (cos(g_angle2) > 0){
    gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
              g_distance * sin(g_angle2),
              g_distance * cos(g_angle2) * cos(g_angle1),
              GRID_SIZE_X/2.0, GRID_SIZE_Y/2.0, 0.0,
              0.0, 1.0, 0.0);
  } else {
    gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
              g_distance * sin(g_angle2),
              g_distance * cos(g_angle2) * cos(g_angle1),
              GRID_SIZE_X/2.0, GRID_SIZE_Y/2.0, 0.0
              , 0.0, -1.0, 0.0);
  }
  
  gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);

  glRotatef(g_angle1*3, 1.0, 0.0, 0.0);
  glRotatef(g_angle2*3, 0.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  Point p = {0.0, 0.0, 0.0};
  Point q = {1.0, 0.0, 0.0};
  draw_grid();
  
  draw_lifegame();
  //draw_cube(p, lightblue);
  //draw_cube(q, red);
  glDisable(GL_DEPTH_TEST);

  glutSwapBuffers();
}


// 背景にグリッドを生成
void draw_grid(){
    GLdouble pointO[] = {0.0, 0.0, -GRID_OFFSET};
    GLdouble pointA[] = {GRID_SIZE_X, 0.0, -GRID_OFFSET};
    for (int i = 0; i <= GRID_SIZE_Y; i++){
        // Lineの太さは0.5が最小...?
        glLineWidth(0.5);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex3dv(pointO);
        glVertex3dv(pointA);
        glEnd();
        pointO[1] += 1.0;
        pointA[1] += 1.0;
    }
    GLdouble pointB[] = {0.0, 0.0, -GRID_OFFSET};
    GLdouble pointC[] = {0.0, GRID_SIZE_Y, -GRID_OFFSET};
    for (int i = 0; i <= GRID_SIZE_X; i++){
        glLineWidth(0.5);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex3dv(pointB);
        glVertex3dv(pointC);
        glEnd();
        pointB[0] += 1.0;
        pointC[0] += 1.0;
    }
}

void glut_idle(){
  static int counter = 0;

  if(counter == 0){
    draw_lifegame();
    draw_cube(Point(0.0, 0.0, 0.0), red);
  }

  counter++;
  if(counter > 100) counter = 0;

  glutPostRedisplay();
}

void timer(int value) {
	draw_lifegame();
  update_cells();
	glutPostRedisplay();
	glutTimerFunc(TIME_SLICE , timer , 0);
}

// Point{x, y, z}を起点にPoint{x+1, y+1, z+1}の立方体を描画する。
void draw_cube(Point p, GLdouble cube_color[]){

  // 1.0だと同一平面上に描画することになり、いろいろあれなので微小区間だけ縮めた
  GLdouble pointO[] = {p.x, p.y, p.z};
  GLdouble pointA[] = {p.x, p.y+0.999, p.z};
  GLdouble pointB[] = {p.x, p.y+0.999, p.z+0.999};
  GLdouble pointC[] = {p.x, p.y, p.z+0.999};
  GLdouble pointD[] = {p.x+0.999, p.y, p.z};
  GLdouble pointE[] = {p.x+0.999, p.y+0.999, p.z};
  GLdouble pointF[] = {p.x+0.999, p.y+0.999, p.z+0.999};
  GLdouble pointG[] = {p.x+0.999, p.y, p.z+0.999};
  

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointE);
  glVertex3dv(pointD);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointE);
  glVertex3dv(pointA);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glVertex3dv(pointE);
  glEnd();
}

void draw_lifegame(){
    for (int y = 0; y < GRID_SIZE_Y; y++){
        for (int x = 0; x < GRID_SIZE_X; x++){
            if (cell[x][y]){
                draw_cube(Point((double)x, (double)y, 0.0), lightblue);
            }
        }
    }
}

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



int count_adjacent_cells(int x, int y){
  int n = 0;
  int dx, dy;
  for (dx = x - 1; dx <= x + 1; dx++) {
    for (dy = y - 1; dy <= y + 1; dy++) {
      if (dx == x && dy == y) continue;
      n += (int)cell[cycle_x(dx)][cycle_y(dy)];
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

int cycle_y(int y){        //境界を消去
  if (y >= GRID_SIZE_Y){
    return y - GRID_SIZE_Y;
  } else if (y < 0){
    return y + GRID_SIZE_Y;
  } else{
    return y;
  }
}

int cycle_x(int x){          //境界を消去
  if (x >= GRID_SIZE_X){
    return x - GRID_SIZE_X;
  } else if (x < 0){
    return x + GRID_SIZE_X;
  } else {
    return x;
  }
}