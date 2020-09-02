#pragma once

// #include <vector>
#include <QMainWindow>
#include <glm/vec2.hpp>
// #include "rayTracingCamera.h"
// #include "geometryObject.h"
// #include "lightSource.h"
// #include "Utils.h"

struct RenderParams {
  glm::uvec2 resolution;
  int multi_sampling;
  int image_scale_ratio;
};

class QPushButton;
class QLineEdit;

namespace ray_tracing {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  // void RenderImage(const QTInputParam&, vector<GeometryObject*> &scene, RayTracingCameraClass* camera, vector<LightBase*> &light);

  // int RayHitTest(RayClass* ray, vector<GeometryObject*> &scene, vector<LightBase*> &light, RayHitObjectRecord &record, float lightDis = MYINFINITE);

  // glm::vec3 calColorOnHitPoint(RayHitObjectRecord &record, vector<GeometryObject*> &scene, vector<LightBase*> &light, int level);

private slots:
	void ChooseSceneFile();
	void RenderScene();

private:
  void InitUI();

	QPushButton *scene_file_button_;
	QLineEdit *scene_file_text_;
  QPushButton *render_button_;

  std::vector<QLineEdit*> cam_pos_vec_;
  std::vector<QLineEdit*> cam_lookat_vec_;

  std::vector<QLineEdit*> resolution_vec_;
  QLineEdit *multi_sampling_;
  QLineEdit *scale_ratio_;

  // void RenderPixels(RayTracingCameraClass* camera, int row, int start, int end, vector<glm::vec3> &pixelList,
  //   vector<GeometryObject*> &scene, vector<LightBase*> &light);
};

}
