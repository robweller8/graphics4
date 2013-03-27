#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "./common.h"
#include "./bb.h"
#include "./mesh.h"
#include "./io.h"
#include "./texture.h"

using namespace std;
#define PI 3.14159265

Mesh mesh;

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

Vec3f eye = Vec3f::makeVec(2, 2, 5);
Vec3f center = Vec3f::makeVec(0, 0, 0);
Vec3f up = Vec3f::makeVec(0, 1, 0);

Point3 mouse_current = Point3(0, 0, 0);
Point3 mouse_last = Point3(0, 0, 0);
bool mouse_motion = false;
bool zoom = false;
GLfloat yzoom = 0;
GLfloat xnormal, ynormal, znormal;

// window parameters
int window_width = 800, window_height = 600;
float window_aspect = window_width / static_cast<float>(window_height);

bool scene_lighting;

void SetCamera() {
  gluLookAt(eye[0], eye[1], eye[2],
            center[0], center[1], center[2],
            up[0], up[1], up[2]);
}

void normalize(Point3* point) {
  point->x = 2 * point->x / window_width - 1;
  point->y = -1 * (2 * point->y / window_height - 1);
  GLfloat ztemp = point->x * point->x + point->y * point->y;
  if (ztemp < 2)
    point->z = sqrt(1 - ztemp);
  else
    point->z = 0;
}

Point3 nCross(Point3* a, Point3* b) {
  Point3 c = Point3();
  c.x = a->y * b->z - a->z * b->y;
  c.y = a->z * b->x - a->x * b->z;
  c.z = a->x * b->y - a->y * b->x;
  GLfloat magnitude = sqrt(c.x * c.x + c.y * c.y + c.z * c.z);
  c.x = c.x/magnitude;
  c.y = c.y/magnitude;
  c.z = c.z/magnitude;
  return c;
}

GLfloat nDot(Point3* a, Point3* b) {
  GLfloat result = a->x * b->x + a->y * b->y + a->z * b->z;
  GLfloat amag = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
  GLfloat bmag = sqrt(b->x * b->x + b->y * b->y + b->z * b->z);
  result = result / (amag * bmag);
  return result;
}

void ZoomIn() {
  cout << "&&&&&&zoomin!!!" << endl;
  eye[0] = eye[0]*0.99;
  eye[1] = eye[1]*0.99;
  eye[2] = eye[2]*0.99;
}

void ZoomOut() {
  cout << "zoomout!!!" << endl;
  eye[0] = eye[0]*1.01;
  eye[1] = eye[1]*1.01;
  eye[2] = eye[2]*1.01;
}

void Rotate(Point3* a, Point3* b) {
  if (a->x != b->x || a->y != b->y || a->z != b->z) {
    Point3 c = nCross(a, b);
    GLfloat sint = sin(acos(nDot(a, b)));
    GLfloat cost = nDot(a, b);
    GLfloat pcos = 1 - cost;
    eye[0]= eye[0]*pcos*c.x*c.x+cost +
              eye[1]*pcos*c.x*c.y+sint*c.z +
                eye[2]*pcos*c.x*c.z-sint*c.y;

    eye[1]= eye[0]*pcos*c.x*c.y-sint*c.z +
              eye[1]*pcos*c.y*c.y+cost +
                eye[2]*pcos*c.y*c.z+sint*c.x;

    eye[2]= eye[0]*pcos*c.z*c.x+sint*c.y +
              eye[1]*pcos*c.z*c.y-sint*c.x +
                eye[2]*pcos*c.z*c.z+cost;
  }
}

void Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0, window_aspect, 1, 1500);

  // TODO call gluLookAt such that mesh fits nicely in viewport.
  // mesh.bb() may be useful.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetCamera();

  // TODO set up lighting, material properties and render mesh.
  // Be sure to call glEnable(GL_RESCALE_NORMAL) so your normals
  // remain normalized throughout transformations.

  // You can leave the axis in if you like.
  glDisable(GL_LIGHTING);
  glLineWidth(4);
  DrawAxis();
  glEnable(GL_LIGHTING);

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
      int ypos = window_height - y;
      mouse_last = Point3(x, y, 0);
      normalize(&mouse_last);
      cout << "eye: " << eye[0] << " " << eye[1] << " " << eye[2] << endl;
    } else {
      mouse_motion = false;
      mouse_last = mouse_current;
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
  if (mouse_motion) {
    mouse_current = Point3(x, y, 0);
    normalize(&mouse_current);
    Rotate(&mouse_last, &mouse_current);
    // mouse_last = mouse_current;
  } else if (zoom) {
    if (y - yzoom > 0) {
      ZoomIn();
      yzoom = y;
    } else {
      ZoomOut();
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

  glutMainLoop();

  return 0;
}
