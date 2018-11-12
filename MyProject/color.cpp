#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <random>
#include <string.h>
#include <vector>
#include <iostream>
#include <thread>



void rgb2hsv(GLfloat rgb_color[], GLfloat hsv_color[]){
  float rgb_max, rgb_min;
  float diff;
  float base;
  float div1;
  float div2;

  rgb_max = std::max(std::max(rgb_color[0], rgb_color[1]), rgb_color[2]);
  rgb_min = std::min(std::min(rgb_color[0], rgb_color[1]), rgb_color[2]);
  if (rgb_max == rgb_color[0])
  {
    diff = rgb_color[1] - rgb_color[2]; // G-B
    base = (rgb_color[1] < rgb_color[2]) ? 360.0f : 0.0f;
  }
  if (rgb_max == rgb_color[1])
  {
    diff = rgb_color[2] - rgb_color[0]; // B-R
    base = 120.0f;
  }
  if (rgb_max == rgb_color[2])
  {
    diff = rgb_color[0] - rgb_color[1]; // R-G
    base = 240.0f;
  }

  div1 = (rgb_max == rgb_min) ? 1.0f : (rgb_max - rgb_min);
  div2 = (rgb_max > 0.0f) ? rgb_max : 1.0f;
  hsv_color[0] = 60.0f * diff / div1 + base;
  hsv_color[1] = (rgb_max - rgb_min) / div2;
  hsv_color[2] = rgb_max;
}

void hsv2rgb(GLfloat hsv_color[], GLfloat rgb_color[]){
   int Hi;
   float f;
   float p;
   float q;
   float t;
   Hi = fmod(floor(hsv_color[0] / 60.0f), 6.0f);
   f = hsv_color[0] / 60.0f - Hi;
   p = hsv_color[2] * (1.0f - hsv_color[1]);
   q = hsv_color[2] * (1.0f - f * hsv_color[1]);
   t = hsv_color[2] * (1.0f - (1.0f - f) * hsv_color[1]);
   if (Hi == 0)
   {
     rgb_color[0] = hsv_color[2];
     rgb_color[1] = t;
     rgb_color[2] = p;
   }
   if (Hi == 1)
   {
     rgb_color[0] = q;
     rgb_color[1] = hsv_color[2];
     rgb_color[2] = p;
   }
   if (Hi == 2)
   {
     rgb_color[0] = p;
     rgb_color[1] = hsv_color[2];
     rgb_color[2] = t;
   }
   if (Hi == 3)
   {
     rgb_color[0] = p;
     rgb_color[1] = q;
     rgb_color[2] = hsv_color[2];
   }
   if (Hi == 4)
   {
     rgb_color[0] = t;
     rgb_color[1] = p;
     rgb_color[2] = hsv_color[2];
   }
   if (Hi == 5)
   {
     rgb_color[0] = hsv_color[2];
     rgb_color[1] = p;
     rgb_color[2] = q;
   }
 }

 int main(void)
 {
     GLfloat hsv_color[4] = {0.0, 0.72, 1.0};
     GLfloat rgb_colors[4][4];

     for (int i = 0; i < 4; i++)
     {
         hsv_color[0] = 60.0 * (float)(i) / (float)(4);
         std::cout << hsv_color[0] << std::endl;
         hsv2rgb(hsv_color, rgb_colors[i]);
         std::cout << rgb_colors[i][0] << ":"<<rgb_colors[i][1] << ":"<<rgb_colors[i][2] << std::endl;
     }
 }