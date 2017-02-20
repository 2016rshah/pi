#include <glut.h>
long window_x_size = 500;
long window_y_size = 500;
void bg_drawrect(long _x, long _y, long _w, long _h){
    float x = _x;
    float y = _y;
    glBegin(GL_POLYGON);
    glVertex3f(x / window_x_size, y, 0.0f);
    glVertex3f((x + _w) / window_x_size, y, 0.0f);
    glVertex3f((x + _w) / window_x_size, (y + _h) / window_y_size, 0.0f);
    glVertex3f(x / window_x_size, (y + _h) / window_y_size, 0.0f);
    glEnd();
    glFlush();
}
void bg_setcolor(long _r, long _g, long _b){
    float r = _r / 255.0f;
    float g = _g / 255.0f;
    float b = _b / 255.0f;
    glColor3f(r, g, b);
}
void bg_setclearcolor(long _r, long _g, long _b){
    float r = _r / 255.0f;
    float g = _g / 255.0f;
    float b = _b / 255.0f;
    glClearColor(r, g, b, 0.0f);
}
void bg_setupwindow(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //We are going to set our bounds in various directions.
    glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
}
