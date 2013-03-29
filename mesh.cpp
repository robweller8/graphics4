#include <iostream>
#include <vector>

#include "./common.h"
#include "./mesh.h"

using namespace std;

// vector<Vec3f> vertices;
// vector<Vec3f> textVertices;
// vector< vector<int> > polygons;  // vector of vertices
// vector< vector<int> > textIndices;  // vector of textIndices
// vector< vector<int> > PolygonsIncidentToVertex;

Mesh::Mesh() {
  _cur_mtl = -1;
}

// This will be called by the obj parser
void Mesh::AddVertex(const Vec3f& v) {
  // TODO
  // cout << v[0] << " " << v[1] << " " << v[2] << endl;

  vertices.push_back(v);

  // updates the bounding box
  _bb(v);
}

// This will be called by the obj parser
void Mesh::AddTextureVertex(const Vec3f& v) {
  // TODO
  textVertices.push_back(v);
}

// p is the list of indices of vertices for this polygon.  For example,
// if p = {0, 1, 2} then the polygon is a triangle with the zeroth, first and
// second vertices added using AddVertex.
//
// pt is the list of texture indices for this polygon, similar to the
// actual vertices described above.
void Mesh::AddPolygon(const std::vector<int>& p, const std::vector<int>& pt) {
  // TODO
  polygons.push_back(p);
  textIndices.push_back(pt);

  // updates the poly2mat map
  _polygon2material.push_back(_cur_mtl);
  // cout << _polygon2material.size() << endl;
  // cout << _cur_mtl << endl;
}

void Mesh::render() {
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glEnable(GL_COLOR_MATERIAL);
  // glPolygonMode(GL_BACK, GL_FILL);
  // glPolygonMode(GL_FRONT, GL_FILL);
  // glColor3f(0, 0, 0);
  // glLineWidth(1);
  for (int i = 0; i < polygons.size(); i++) {
    int mtl_index = polygon2material(i);
    Material mat = material(mtl_index);
    // cout << mat.ambient()[0] << " " << mat.ambient()[1] << " " <<
    //   mat.ambient()[2] << endl;
    // cout << mat.diffuse()[0] << " " << mat.diffuse()[1] << " " <<
    //   mat.diffuse()[2] << endl;

    GLfloat ambient[3] = {mat.ambient()[0], mat.ambient()[1],
      mat.ambient()[2]};
    GLfloat diffuse[3] = {mat.diffuse()[0], mat.diffuse()[1],
      mat.diffuse()[2]};
    GLfloat specular[3] = {mat.specular()[0], mat.specular()[1],
      mat.specular()[2]};

    // GLfloat test1[3] = {1.0, 1.0, 1.0};
    // GLfloat test2[3] = {1.0, 1.0, 0.0};
    // GLfloat test3[3] = {0.0, 0.0, 0.0};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.specular_coeff());

    // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, test1);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, test2);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, test3);

    glBindTexture(GL_TEXTURE_2D, mat.texture_id());

    glBegin(GL_POLYGON);
    for (int j = 0; j < polygons[i].size()  ; j++) {
      glNormal3f(normVectors[polygons[i][j]][0],
        normVectors[polygons[i][j]][1],
        normVectors[polygons[i][j]][2]);
      glTexCoord2d(textVertices[polygons[i][j]][0],
        textVertices[polygons[i][j]][1]);
      glVertex3f(vertices[polygons[i][j]][0], vertices[polygons[i][j]][1],
        vertices[polygons[i][j]][2]);
    }
    glEnd();

    // Draw normals (different way)
    // GLfloat test4[3] = {1.0, 0.0, 0.0};
    // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, test4);
    // glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    // glBegin(GL_LINE);
    // for (int j = 0; j < polygons[i].size()  ; j++) {
    //   glVertex3f(verticesNormals[polygons[i][j]][0],
    //     verticesNormals[polygons[i][j]][1],
    //     verticesNormals[polygons[i][j]][2]);
    //   // cout << verticesNormals[polygons[i][j]][0] << " "
    //   //   << verticesNormals[polygons[i][j]][1] << " "
    //   //   << verticesNormals[polygons[i][j]][2] << endl;

    //   glVertex3f(vertices[polygons[i][j]][0], vertices[polygons[i][j]][1],
    //     vertices[polygons[i][j]][2]);
    // }
    // glEnd();
  }
  // cout << polygons.size() << endl;
  // cout << polygons[0].size() << endl;
  // glutPostRedisplay();
}

void Mesh::drawVertexNormals() {
  glColor3f(1, 0, 0);
  glLineWidth(1);
  glBegin(GL_LINES);
  for (int i = 0; i < verticesNormals.size(); i++) {
    glVertex3f(vertices[i][0], vertices[i][1], vertices[i][2]);
    glVertex3f(verticesNormals[i][0], verticesNormals[i][1],
      verticesNormals[i][2]);
  }
  glEnd();
}

// Computes a normal for each vertex.
void Mesh::compute_normals() {
  // TODO don't forget to normalize your normals!
  // cout << "NORMALS" << endl;
  // determine which polygons are incident to which vertices
  for (int k = 0; k < vertices.size(); k++) {
    vector <int> poly;
    for (int i = 0; i < polygons.size(); i++) {
      for (int j = 0; j < 3; j++) {
        // k = which vertex
        // i = which polygon
        if (k == polygons[i][j])  // vertex in the i polygon
          poly.push_back(i);
      }
    }
    PolygonsIncidentToVertex.push_back(poly);
  }
  // cout << vertices.size() << endl;
  // cout << PolygonsIncidentToVertex.size() << endl;
  // for (int i = 0; i < PolygonsIncidentToVertex[1575].size(); i++) {
  //   cout << PolygonsIncidentToVertex[1575][i] << endl;
  // }

  // index through vertices
  for (int i = 0; i < PolygonsIncidentToVertex.size(); i++) {
    Vec3f vertexNormal = {0.0, 0.0, 0.0};

    // index through polygons per vertex
    for (int j = 0; j < PolygonsIncidentToVertex[i].size(); j++) {
      // compute normal for each polygon
      int currentPolygon = PolygonsIncidentToVertex[i][j];

      // vertices for polygon
      Vec3f firstVertex = vertices[polygons[currentPolygon][0]];
      Vec3f secondVertex = vertices[polygons[currentPolygon][1]];
      Vec3f thirdVertex = vertices[polygons[currentPolygon][2]];

      // two vectors in polygon
      Vec3f firstVector = normVecDiff(firstVertex, secondVertex);
      Vec3f secondVector = normVecDiff(thirdVertex, secondVertex);

      // normal for polygon
      Vec3f currentNormal = normCrossProd(secondVector, firstVector);
      vertexNormal[0] = vertexNormal[0] + currentNormal[0];
      vertexNormal[1] = vertexNormal[1] + currentNormal[1];
      vertexNormal[2] = vertexNormal[2] + currentNormal[2];
      vertexNormal = normalize(vertexNormal);
    }

    vertexNormal[0] = vertexNormal[0] + vertices[i][0];
    vertexNormal[1] = vertexNormal[1] + vertices[i][1];
    vertexNormal[2] = vertexNormal[2] + vertices[i][2];

    // push normal to appropriate vertex
    verticesNormals.push_back(vertexNormal);
    Vec3f normVector = normVecDiff(vertexNormal, vertices[i]);
    normVectors.push_back(normVector);

    // cout << vertexNormal[0] << " " << vertexNormal[1] << " " <<
    //   vertexNormal[2] << endl;
  }
}
