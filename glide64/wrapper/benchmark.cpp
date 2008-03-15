#ifndef _WIN32
#include "../winlnxdefs.h"
#include <stdarg.h>
#include <string.h>
#include <SDL/SDL.h>
#endif
#include "glide.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#endif // _WIN32
#include "main.h"
#ifdef _WIN32
#include "wglext.h"
#endif // _WIN32

#ifndef WIN32
BOOL WINAPI QueryPerformanceCounter(PLARGE_INTEGER counter);
#endif

extern int width, height;

void do_benchmarks()
{
  FILE * fp;
  int tw, th;

  fp = fopen("glide3x-benchmark.txt", "w");

  if (!fp)
    goto error;

  glPushAttrib(GL_ALL_ATTRIB_BITS);

//   if (glUseProgramObjectARB)
//     glUseProgramObjectARB(0);
  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(-1, -1, 0);
  glScalef(2, 2, 1);

  int npot, deltex, loop;
  for (npot=0; npot<3; npot++)
    for (deltex=0; deltex<2; deltex++) {
      int maxloop = 100;
      LARGE_INTEGER start;
      for (loop=0; loop<maxloop; loop++)
  {

  if (loop == 5)
    QueryPerformanceCounter(&start);
  

  switch (npot) {
    case 0:
      tw=1; while(tw<width) tw*=2;
      th=1; while(th<height) th*=2;
      break;
    case 1:
      tw = width;
      th = height;
      break;
    case 2:
      tw = 128;
      th = 128;
      break;
  }

  GLuint tex = 1;
  
  if (deltex) {
    glDeleteTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     0, 0, tw, th, 0);
  } else {
    glBindTexture(GL_TEXTURE_2D, tex);
    int w, h, fmt;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &fmt);
    if (w == tw && h == th && fmt == GL_RGB)
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                          0, 0, tw, th);
    else
      glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                       0, 0, tw, th, 0);
  }
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  if (loop>0)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glColor3f(1, 1, 0.5);
  glBegin(GL_QUADS);

  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(0.05f, 0.05f);

  glTexCoord2f(1.0f*width/tw, 0.0f);
  glVertex2f(0.95f, 0.05f);

  glTexCoord2f(1.0f*width/tw, 1.0f*height/th);
  glVertex2f(0.95f, 0.95f);

  glTexCoord2f(0.0f, 1.0f*height/th);
  glVertex2f(0.05f, 0.95f);

  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(0.05f, 0.05f);

  glEnd();

  //grBufferSwap(0);

  }

      LARGE_INTEGER end;
      QueryPerformanceCounter(&end);
      fprintf(fp, "npot %d deltex %d : %gms\n", npot, deltex,
              (end.QuadPart-start.QuadPart)/1000.0f);
  }

  glPopMatrix();
  glPopAttrib();


error:
  if (fp)
    fclose(fp);
}
