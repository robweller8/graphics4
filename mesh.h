#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include <string>

#include "./vec.h"
#include "./bb.h"
#include "./material.h"

// This class represents a mesh, which contains vertices, polygons,
// and material properties for each polygon.
class Mesh {
 public:
  Mesh();

  void AddVertex(const Vec3f& v);
  void AddTextureVertex(const Vec3f& v);
  void AddPolygon(const std::vector<int>& p, const std::vector<int>& pt);
  void render();
  void drawVertexNormals();

  void new_material(int material_idx, const std::string& name) {
    _materials.push_back(Material(name));
    // cout << "MATERIAL" << endl;
    // cout <<_materials.size() << endl;
  }

  void set_cur_material(const string& name) {
    for (int i = 0; i < _materials.size(); ++i) {
      if (_materials[i].name() == name) {
        set_cur_material(i);
      }
    }
    // cout << "set material" << endl;
  }

  void set_cur_material(int cur_mtl) {
    _cur_mtl = cur_mtl;
    // cout << "SET MATERIAL" << endl;
  }

  void set_ambient(int material_idx, const Vec3f& ambient) {
    _materials[material_idx].set_ambient(ambient);
    cout << ambient << endl;
  }

  void set_diffuse(int material_idx, const Vec3f& diffuse) {
    _materials[material_idx].set_diffuse(diffuse);
    cout << diffuse << endl;
  }

  void set_specular(int material_idx, const Vec3f& specular) {
    _materials[material_idx].set_specular(specular);
    cout << specular << endl;
  }

  void set_specular_coeff(int material_idx, const float& coeff) {
    _materials[material_idx].set_specular_coeff(coeff);
    cout << coeff << endl;
  }

  void set_texture(int material_idx, const string& texture) {
    _materials[material_idx].set_texture(texture);
  }

  // TODO - you don't need to modify this function, but it will be useful
  // in your Display() function.
  //
  // Given a polygon index, returns the index of the material
  // that should be used for that polygon.
  int polygon2material(int polygon_idx) const {
    return _polygon2material[polygon_idx];
  }

  const Material& material(int i) const { return _materials[i]; }

  Material& material(int i) { return _materials[i]; }

  const BoundingBox& bb() const { return _bb; }

  int num_materials() const { return _materials.size(); }

  void compute_normals();

  Vec3f normCrossProd(Vec3f v, Vec3f w) {
    Vec3f c;
    c[0] = v[1]*w[2] - v[2]*w[1];
    c[1] = v[2]*w[0] - v[0]*w[2];
    c[2] = v[0]*w[1] - v[1]*w[0];
    float length = sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
    c[0] = c[0]/length;
    c[1] = c[1]/length;
    c[2] = c[2]/length;
    return c;
  }

  Vec3f normVecDiff(Vec3f v, Vec3f w) {
    Vec3f c;
    c[0] = v[0] - w[0];
    c[1] = v[1] - w[1];
    c[2] = v[2] - w[2];
    float length = sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
    c[0] = c[0]/length;
    c[1] = c[1]/length;
    c[2] = c[2]/length;
    return c;
  }

  Vec3f normalize(Vec3f v) {
    Vec3f c;
    float length = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    c[0] = v[0]/length;
    c[1] = v[1]/length;
    c[2] = v[2]/length;
    return c;
  }

 private:
  // TODO add necessary data structures here
  vector<Vec3f> vertices;
  vector<Vec3f> textVertices;
  vector< vector<int> > polygons;  // vector of vertices
  vector< vector<int> > textIndices;  // vector of textIndices
  vector< vector<int> > PolygonsIncidentToVertex;
  vector<Vec3f> verticesNormals;
  vector<Vec3f> normVectors;

  std::vector<Material> _materials;
  std::vector<int> _polygon2material;
  int _cur_mtl;
  BoundingBox _bb;
};

#endif
