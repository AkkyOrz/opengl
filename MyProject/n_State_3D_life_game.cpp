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

// 定数
GLdouble lightblue[] = {0.5, 1.0, 1.0, 1.0}; // color
GLdouble red[] = {1.0, 0.5, 0.5, 1.0};       // color
const int GRID_SIZE_X = 50;             //gridの縦と横
const int GRID_SIZE_Y = 50;
const int GRID_SIZE_Z = 50;
const int BUFSIZE = 1000;
const double GRID_OFFSET = 0.5;
const bool IS_INPUT = false; //true なら　tmp.txtから持ってくる false はランダム
const int TIME_SLICE = 1;
const double INIT_CELL_PROPOTION = 0.05; //初期のcellの割合 0はすべて死滅(のはず)
const double ALPHA = 1.0;

const int N_STATE = 5;
const int birth[]   = {4};
const int survive[] = {4};


const int color_ver = 1;  // 0 は原点から最奥までのグラデーション, 
                          // 1は状態ごとの色分け
                          // 2は単色, 
                          // 3は中心からのグラデーション
static int N_birth;
static int N_survive;


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


// lifegame関連のプロトタイプ宣言
int cycle_x(int x);
int cycle_y(int y);
int cycle_z(int z);
void init_cells();
int count_adjacent_cells(int x, int y, int z);
void update_cells();
bool check_around(int dx, int dy, int dz, int state);

// 物体描画関連のプロトタイプ宣言
void draw_pyramid();
void draw_cube(Point p, GLdouble cube_color[]);
void draw_cube_trans(Point p, GLdouble cube_color[]); 
void draw_grid();
void draw_cubic_line();
void draw_lifegame();


// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 5 * GRID_SIZE_X;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
bool is_stop = false;
int cell[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
int cell_next[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
//double cell_size[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
static int counter = 0;
static int speed = 10;



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
  glClearColor(0.2, 0.2, 0.2, 0.2);         // 背景の塗りつぶし色を指定
  init_cells();
  N_birth = sizeof(birth)/sizeof(*birth);
  N_survive = sizeof(survive)/sizeof(*survive);
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
  case 't':
    is_stop = true;
    break;

  case 's':
    is_stop = false;
    break;

  case 'r':
    init_cells();
    break;

  case 'f':
    if (speed > 1) speed--;
    break;
  
  case 'd':
    speed++;
    break;

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
  gluPerspective(30.0, 1.0, 0.1, g_distance * 10);  // ここでカメラの写せる範囲を指定している

  // つぎにモデル・ビュー変換
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  if (cos(g_angle2) > 0){
    gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
              g_distance * sin(g_angle2),
              g_distance * cos(g_angle2) * cos(g_angle1),
              GRID_SIZE_X/2.0, GRID_SIZE_Y/2.0, GRID_SIZE_Z/2.0,
              0.0, 1.0, 0.0);
  } else {
    gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
              g_distance * sin(g_angle2),
              g_distance * cos(g_angle2) * cos(g_angle1),
              GRID_SIZE_X/2.0, GRID_SIZE_Y/2.0, GRID_SIZE_Z/2.0
              , 0.0, -1.0, 0.0);
  }
  
  gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);

  glRotatef(g_angle1*3, 1.0, 0.0, 0.0);
  glRotatef(g_angle2*3, 0.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  Point p = {0.0, 0.0, 0.0};
  Point q = {1.0, 0.0, 0.0};
  //draw_grid();
  draw_cubic_line();

  draw_lifegame();
  //draw_cube(p, lightblue);
  //draw_cube(q, red);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  glutSwapBuffers();
}


// 背景にグリッドを生成
// gridの3次元はできてない
// 必要になったらやる
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

void draw_cubic_line(){
    GLfloat vertics[8][3] =
        {
            {0.0f, 0.0f, 0.0f},                         // 0
            {GRID_SIZE_X, 0.0f, 0.0f},                  // 1
            {0.0f, GRID_SIZE_Y, 0.0f},                  // 2
            {0.0f, 0.0f, GRID_SIZE_Z},                  // 3
            {GRID_SIZE_X, GRID_SIZE_Y, 0.0f},           // 4
            {0.0f, GRID_SIZE_Y, GRID_SIZE_Z},           // 5
            {GRID_SIZE_X, 0.0f, GRID_SIZE_Z},           // 6
            {GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z},    // 7
        };

    glLineWidth(1.0);
    glColor3f(1.0, 1.0, 1.0);
    for (int i = 0; i < 4; i++){
        glBegin(GL_LINES);
        glVertex3fv(vertics[0]);
        glVertex3fv(vertics[i]);
        glEnd();
    }
    for (int i = 4; i < 7; i++){
        glBegin(GL_LINES);
        glVertex3fv(vertics[7]);
        glVertex3fv(vertics[i]);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex3fv(vertics[1]);
    glVertex3fv(vertics[4]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[2]);
    glVertex3fv(vertics[5]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[3]);
    glVertex3fv(vertics[6]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[1]);
    glVertex3fv(vertics[6]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[2]);
    glVertex3fv(vertics[4]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[3]);
    glVertex3fv(vertics[5]);
    glEnd();
}

/*
void glut_idle(){
  static int counter = 0;

  if(counter == 0){
    update_cells();
    //draw_lifegame();
  }
  if (!is_stop) counter++;
  if(counter > 100) counter = 0;

  glutPostRedisplay();
}
*/

void timer(int value) {
  if(counter % speed == 0){
    update_cells();
    draw_lifegame();
  }
  if (!is_stop) counter++;
  if (counter > 1000) counter = 0;
  
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

// Point{x, y, z}を起点にPoint{x+1, y+1, z+1}の立方体を描画する。
// ただし透過する
void draw_cube_trans(Point p, GLdouble cube_color[]){
// counter 
  // 1.0だと同一平面上に描画することになり、いろいろあれなので微小区間だけ縮めた
  GLdouble pointO[] = {p.x, p.y, p.z};
  GLdouble pointA[] = {p.x, p.y+0.999, p.z};
  GLdouble pointB[] = {p.x, p.y+0.999, p.z+0.999};
  GLdouble pointC[] = {p.x, p.y, p.z+0.999};
  GLdouble pointD[] = {p.x+0.999, p.y, p.z};
  GLdouble pointE[] = {p.x+0.999, p.y+0.999, p.z};
  GLdouble pointF[] = {p.x+0.999, p.y+0.999, p.z+0.999};
  GLdouble pointG[] = {p.x+0.999, p.y, p.z+0.999};
  

  glColor4d(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glEnd();

  glColor4d(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointE);
  glVertex3dv(pointD);
  glEnd();

  glColor4d(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glEnd();

  glColor4d(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointE);
  glVertex3dv(pointA);
  glEnd();

  glColor4d(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glEnd();

  glColor4d(cube_color[0], cube_color[1], cube_color[2], cube_color[3]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glVertex3dv(pointE);
  glEnd();
}


// おそらくswitch文で書き換えるべき

void draw_lifegame(){
  GLdouble point_color[4];
  if (color_ver == 0){ 
    // 原点から再奥までのグラデーション
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]){
            point_color[0] = (double)(x) / GRID_SIZE_X;
            point_color[1] = (double)(y) / GRID_SIZE_Y;
            point_color[2] = (double)(z) / GRID_SIZE_Z;
            point_color[3] = 1.0;
            draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  } else if (color_ver == 1){
    // 状態に応じて色を変える
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]){
            point_color[0] = 0.2 + ((double)(cell[x][y][z]*1.0) / (N_STATE - 1)) * 0.7;
            point_color[1] = 0.2;
            point_color[2] = 0.5;
            point_color[3] = 1.0;
            draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  } else if (color_ver == 2){
    // 単色
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]) {
            draw_cube_trans(Point((double)x, (double)y, double(z)), red);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  } else if (color_ver == 3){
    // 中心からのグラデーション
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]){
            point_color[0] = (double)std::abs(x - GRID_SIZE_X / 2) / GRID_SIZE_X;
            point_color[1] = 0.2;
            point_color[2] = 0.2;
            point_color[3] = 1.0;
            draw_cube_trans(Point((double)x, (double)y, double(z)), red);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  }
}

void init_cells(){
  int x = 0, y;
  char buf[BUFSIZE];
  FILE *fp;
  const char *filename = "tmp.txt";

  
  if (IS_INPUT){ //  仕方がないので z = 0 として初期化を行う
    if ((fp = fopen(filename, "r")) == NULL) {
      printf("error: can't open %s\n", filename);
      return;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
      size_t len = strlen(buf) - 1;
      for (x = 0; x < len; x++){
        if (buf[x] == ' '){
          cell[x][y][0] = 0;
        } else {
          cell[x][y][0] = N_STATE-1;
        }
      }
      y++;
    }
    fclose(fp);
  } else {
    // 乱数に従って最初の分布を生成
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::bernoulli_distribution distribution(INIT_CELL_PROPOTION);
    for (int y = 0; y < GRID_SIZE_Y; y++){
        for (int x = 0; x < GRID_SIZE_X; x++){
          for(int z = 0; z < GRID_SIZE_Z; z++){
            if (distribution(engine)){
                cell[x][y][z] = N_STATE-1;
            } else {
                cell[x][y][z] = 0;
            }
          }
        }
    }
  }
}


// 周り 3^3 - 1 = 26 個を探索する
//intをboolでやる方法がよくわからんけど

// ここのcheck_around()は編集

int count_adjacent_cells(int x, int y, int z){
  int n = 0;
  int dx, dy, dz;
  int state = cell[x][y][z];
  for (dx = x - 1; dx <= x + 1; dx++) {
    for (dy = y - 1; dy <= y + 1; dy++) {
      for (dz = z - 1; dz <= z + 1; dz++){
        if (dx == x && dy == y && dz == z) continue;
        if (check_around(cycle_x(dx), cycle_y(dy), cycle_z(dz), state) == true)
          n++;
      }
    }
  }
  return n;
}


// 自分の状態によって数える周りの状態が変わる
// dx, dy, dz は周りのセル
// state は自分の状態

bool check_around(int dx, int dy, int dz, int state){
  if (state == 0){
    // 新しいセルを発生させられるかどうか
    return cell[dx][dy][dz] >= N_STATE - 1;   
  } else {
    // 自分の状態より「良い」状態のセルかどうか
    return cell[dx][dy][dz] >= state;
  }
}

bool check_birth(int x, int y, int z, int n){
    bool birth_val = false;
    if (cell[x][y][z] > 0) return false;
    for (int i = 0; i < N_birth; i++){
      if (birth[i] == n){
        birth_val = true;
        break;
      }
    }
    return birth_val;
}

bool check_survive(int x, int y, int z, int n){
  bool survive_val = false;
  // 自分がsurviveじゃないなら関係ない。
  if (cell[x][y][z] == 0) return false;
  for (int i = 0; i < N_survive; i++){
    if (survive[i] == n){
      survive_val = true;
      break;
    }
  }
  return survive_val;
}

// この関数によって次の状態の細胞の生死を決定するが、、、
// 3Dのあれは想像できないので、今は適当な数字を考える。
void update_cells(){
  int x, y, z;

  for (x = 0; x < GRID_SIZE_X; x++) {
    for (y = 0; y < GRID_SIZE_Y; y++) {
      for (z = 0; z < GRID_SIZE_Z; z++){
        cell_next[x][y][z] = 0;
        const int n = count_adjacent_cells(x, y, z);
        if (check_survive(x,y,z,n)){           // survive
          cell_next[x][y][z] = cell[x][y][z];
          if (cell_next[x][y][z] > 0) 
            cell_next[x][y][z]--;
        } else if (check_birth(x,y,z,n)){ // birth
          cell_next[x][y][z] = N_STATE - 1;     
        } else {
          cell_next[x][y][z] = 0;
        }
      }
    }
  }

  for (x = 0; x < GRID_SIZE_X; x++) {
    for (y = 0; y < GRID_SIZE_Y; y++) {
      for (z = 0; z < GRID_SIZE_Z; z++){
        cell[x][y][z] = cell_next[x][y][z];
      }
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

int cycle_z(int z){        //境界を消去
  if (z >= GRID_SIZE_Z){
    return z - GRID_SIZE_Z;
  } else if (z < 0){
    return z + GRID_SIZE_Z;
  } else{
    return z;
  }
}

