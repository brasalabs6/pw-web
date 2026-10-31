#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
static GLenum mode;
static int n, errors;
static double *v[3], area;
static void begin(GLenum m) { mode=m; n=0; }
static void vertex(void *p) {
 double *q=p;
 if (mode==GL_TRIANGLES) {v[n++]=q; if(n<3)return; n=0;}
 else if(mode==GL_TRIANGLE_FAN) {if(n<2){v[n++]=q;return;}v[2]=q;}
 else if(mode==GL_TRIANGLE_STRIP) {if(n<2){v[n++]=q;return;}v[2]=q;}
 else {errors++;return;}
 area+=fabs((v[1][0]-v[0][0])*(v[2][1]-v[0][1])-(v[1][1]-v[0][1])*(v[2][0]-v[0][0]))/2;
 if(mode==GL_TRIANGLE_FAN)v[1]=v[2];
 if(mode==GL_TRIANGLE_STRIP){v[0]=v[1];v[1]=v[2];}
}
static void end(void) {}
static void error(GLenum e) {errors++;fprintf(stderr,"GLU error %u\n",e);}
int main(void) {
 double outer[4][3]={{0,0,0},{4,0,0},{4,4,0},{0,4,0}};
 double hole[4][3]={{1,1,0},{1,3,0},{3,3,0},{3,1,0}};
 GLUtesselator *t=gluNewTess(); if(!t)return 1;
 gluTessCallback(t,GLU_TESS_BEGIN,(_GLUfuncptr)begin); gluTessCallback(t,GLU_TESS_VERTEX,(_GLUfuncptr)vertex);
 gluTessCallback(t,GLU_TESS_END,(_GLUfuncptr)end); gluTessCallback(t,GLU_TESS_ERROR,(_GLUfuncptr)error);
 gluTessProperty(t,GLU_TESS_WINDING_RULE,GLU_TESS_WINDING_ODD);
 gluTessBeginPolygon(t,0);gluTessBeginContour(t);for(int i=0;i<4;i++)gluTessVertex(t,outer[i],outer[i]);gluTessEndContour(t);
 gluTessBeginContour(t);for(int i=0;i<4;i++)gluTessVertex(t,hole[i],hole[i]);gluTessEndContour(t);gluTessEndPolygon(t);gluDeleteTess(t);
 printf("area %.2f errors %d\n",area,errors);return errors || fabs(area-12)>1e-6;
}
