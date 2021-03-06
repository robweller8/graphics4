#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include "./common.h"
#include "./bb.h"
#include "./mesh.h"
#include "./io.h"
#include "./texture.h"

using namespace std;
#define PI 3.14159265

Mesh mesh;
BoundingBox bb;

GLuint* texture_ids;

class Point3 {
  public:
    GLfloat x, y, z;
    Point3();  // default constructor
    Point3(GLfloat _x, GLfloat _y, GLfloat _z);  // constructor
};

Point3::Point3() {  // default constructor
  x = 0;
  y = 0;
  z = 0;
}

Point3::Point3(GLfloat _x, GLfloat _y, GLfloat _z) {  // constructor
  x = _x;
  y = _y;
  z = _z;
}

Point3 eye = Point3(0, 0, 200);
Point3 eyecopy = Point3(2, 2, 5);
Vec3f center = Vec3f::makeVec(0, 0, 0);
Vec3f up = Vec3f::makeVec(0, 1, 0);
Vec3f mouse_current = Vec3f::makeVec(0, 0, 0);
Vec3f mouse_last = Vec3f::makeVec(0, 0, 0);
GLfloat theta = 0;
Vec3f cross = Vec3f::makeVec(0, 0, 0);
bool mouse_motion = false;
bool zoom = false;
GLfloat yzoom = 0;
GLfloat saved[16]= {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                       0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f};
GLfloat axis[3];
GLdouble x, y, z;

// window parameters
int window_width = 800, window_height = 600;
float window_aspect = window_width / static_cast<float>(window_height);

bool scene_lighting;

void SetCamera() {
  gluLookAt(eye.x, eye.y, eye.z,
            0.0f, 0.0f, 0.0f,
            up[0], up[1], up[2]);
}

void zcalculate(Vec3f* point) {
  (*point)[0] = 2 * (*point)[0] / window_width - 1;
  (*point)[1] = -1 * (2 * (*point)[1] / window_height -1);
  (*point)[1] = (*point)[1]/window_aspect;
  GLfloat ztemp = (*point)[0] * (*point)[0] + (*point)[1] * (*point)[1];
  if (ztemp < 1) {
    (*point)[2] = sqrt(1 - ztemp);
  } else {
    (*point)[2] = 0;
  }
  (*point).unit();
}

void ZoomIn() {
  glLoadMatrixf(saved);
  eye.x = eye.x*0.99;
  eye.y = eye.y*0.99;
  eye.z = eye.z*0.99;
  glGetFloatv(GL_MODELVIEW_MATRIX, saved);
}

void ZoomOut() {
  glLoadMatrixf(saved);
  eye.x = eye.x*1.01;
  eye.y = eye.y*1.01;
  eye.z = eye.z*1.01;
  glGetFloatv(GL_MODELVIEW_MATRIX, saved);
}

void Rotate(Vec3f* a, Vec3f* b) {
    (*a) = (*a).unit();
    (*b) = (*b).unit();
    cross = (*a)^(*b);
    theta = 180*acos(((*a)*(*b))/(((*a).norm())*((*b).norm())))/PI;
}

void Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, window_aspect, 1, 1500);

  // TODO call gluLookAt such that mesh fits nicely in viewport.
  // mesh.bb() may be useful. !!!!DONE IN MAIN!!!!

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  SetCamera();

  if (theta != 0)
    glRotatef(theta, cross[0], cross[1], cross[2]);
  glMultMatrixf(saved);

  // TODO set up lighting, material properties and render mesh.
  // Be sure to call glEnable(GL_RESCALE_NORMAL) so your normals
  // remain normalized throughout transformations.

  // You can leave the axis in if you like.
  glDisable(GL_LIGHTING);
  // mesh.drawVertexNormals();
  glLineWidth(4);
  DrawAxis();
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  GLfloat l1[] = {150.0f, 150.0f, 150.0f, 1.0f};
  GLfloat l2[] = {0.0f, 10.0f, 0.0f, 1.0f};
  if (scene_lighting) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(l1[0], l1[1], l1[2]);
    GLfloat poszero[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, poszero);
    glTranslatef(-1*l1[0], -1*l1[1], -1*l1[2]);

    // glTranslatef(l2[0], l2[0], l2[0]);
    // GLfloat pos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    // glLightfv(GL_LIGHT0, GL_POSITION, poszero);
    // glTranslatef(-1*l2[0], -1*l2[1], -1*l2[2]);

    glPopMatrix();

  } else {
    glLightfv(GL_LIGHT0, GL_POSITION, l1);
    // glLightfv(GL_LIGHT0, GL_POSITION, l2);
  }

  // GLfloat lightColor0[] = {0.5f, 0.5f, 0.5f, 1.0f};
  // GLfloat lightPos0[] = {-300.0f, 300.0f, 300.0f, 1.0f};
  // glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
  // glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
  glPushMatrix();
  glTranslatef(-center[0], -center[1], -center[2]);
  mesh.render();
  glTranslatef(center[0], center[1], center[2]);
  glPopMatrix();


  glFlush();
  glutSwapBuffers();
}

void PrintMatrix(GLfloat* m) {
  cout.precision(2);
  int w = 6;
  for (int i = 0; i < 4; ++i) {
    cout << setprecision(2) << setw(w) << m[i] << " "
         << setprecision(2) << setw(w) << m[i+4] << " "
         << setprecision(2) << setw(w) << m[i+8] << " "
         << setprecision(2) << setw(w) << m[i+12] << " "
         << endl;
  }
  cout << endl;
}

void PrintMatrix(GLint matrix) {
  GLfloat m[16];
  glGetFloatv(matrix, m);
  PrintMatrix(m);
}

void PrintMatrix() {
  PrintMatrix(GL_MODELVIEW_MATRIX);
}

void LoadMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
  glLoadMatrixf(m);
}

void MultMatrix(GLfloat* m) {
  // transpose to column-major
  for (int i = 0; i < 4; ++i) {
    for (int j = i; j < 4; ++j) {
      swap(m[i*4+j], m[j*4+i]);
    }
  }
  glMultMatrixf(m);
}

void Init() {
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // resize the window
  window_aspect = window_width/static_cast<float>(window_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(40.0, window_aspect, 1, 1500);
}

void DrawAxis() {
  const Vec3f c = {0, 0, 0};
  const float L = 1;
  const Vec3f X = {L, 0, 0}, Y = {0, L, 0}, Z = {0, 0, L};

  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glVertex3fv(c.x);
  glVertex3fv((c+X).x);
  glColor3f(0, 1, 0);
  glVertex3fv(c.x);
  glVertex3fv((c+Y).x);
  glColor3f(0, 0, 1);
  glVertex3fv(c.x);
  glVertex3fv((c+Z).x);
  glEnd();
}

void MouseButton(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      mouse_motion = true;
      mouse_last[0] = x;
      mouse_last[1] = y;
      mouse_last[2] = 0;
      zcalculate(&mouse_last);
    } else {
      mouse_motion = false;
      mouse_last = mouse_current;
      glPushMatrix();
      glLoadIdentity();
      glRotatef(theta, cross[0], cross[1], cross[2]);
      glMultMatrixf(saved);
      glGetFloatv(GL_MODELVIEW_MATRIX, saved);
      glPopMatrix();
      theta = 0;
    }
    } else {
        if (state == GLUT_DOWN) {
          zoom = true;
          yzoom = y;
        } else {
          zoom = false;
        }
  }
}
void MouseMotion(int x, int y) {
  mouse_current[0] = x;
  mouse_current[1] = y;
  mouse_current[2] = 0;
  zcalculate(&mouse_current);
  if (mouse_motion) {
    Rotate(&mouse_last, &mouse_current);
  } else if (zoom) {
    if (y - yzoom > 0) {
      ZoomOut();
      yzoom = y;
    } else {
      ZoomIn();
      yzoom = y;
    }
  }
  glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
    case 27:  // esc
      exit(0);
      break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << endl;
    cout << "Usage: ./viewer (filename.obj | -s) [-l]" << endl;
    cout << endl;
    cout << "To load data/test.obj: ./viewer data/test.obj" << endl;
    cout << "To load a custom scene: ./viewer -s" << endl;
    cout << endl;
    return 0;
  }

  // Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(window_width, window_height);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Object viewer");
  glutMouseFunc(MouseButton);
  glutMotionFunc(MouseMotion);
  glutKeyboardFunc(Keyboard);
  glutDisplayFunc(Display);

  Init();

  if (string(argv[1]) == "-s") {
    cout << "Create scene" << endl;
  } else {
    string filename(argv[1]);
    cout << filename << endl;

    // Detect whether to fix the light source(s) to the camera or the world
    scene_lighting = false;
    if (argc > 2 && string(argv[2]) == "-l") {
      scene_lighting = true;
    }

    // Parse the obj file, compute the normals, read the textures

    ParseObj(filename, mesh);
    mesh.compute_normals();
    texture_ids = new GLuint[mesh.num_materials()];
    glGenTextures(mesh.num_materials(), texture_ids);

    for (int i = 0; i < mesh.num_materials(); ++i) {
      Material& material = mesh.material(i);
      material.LoadTexture(texture_ids[i]);
    }
  }
  bb = mesh.bb();
  center = bb.center();
  GLfloat tan = std::tan(20*PI/180);
  GLfloat width = 1.3*(0.5*bb.zdim());
  GLfloat length = 1.3*(0.5*bb.xdim());
  GLfloat height = 1.3*(0.5*bb.ydim());
  GLfloat distance;
  if (height >= length && height >= width) {
    distance = height/tan;
    if (length >= width) {
      eye.x = 0;
      eye.y = 0;
      eye.z = distance;
    } else {
      eye.x = distance;
      eye.y = 0;
      eye.z = 0;
    }
  } else if (length >= width) {
    distance = length/tan;
    eye.x = 0;
    eye.y = 0;
    eye.z = distance;
  } else {
    distance = width/tan;
    eye.x = distance;
    eye.y = 0;
    eye.z = 0;
  }
  glutMainLoop();

  return 0;
}




















