#pragma once

// #include <vector>
#include <QMainWindow>
// #include "rayTracingCamera.h"
// #include "geometryObject.h"
// #include "lightSource.h"
// #include "Utils.h"

// // struct for qt UI input paramschrome
// struct QTInputParam
// {
//   int resolutionW;
//   int resolutionH;
//   int antiAliasingLevel;
//   int imageScaleRatio;
// };

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

private:
  void InitUI();

	QPushButton *scene_file_button_;
	QLineEdit *scene_file_text_;

  // // scene is loaded from text, so we need to deal with the line data
  // void processSceneData(vector<GeometryObject*> &scene, QString line);

  // void RenderPixels(RayTracingCameraClass* camera, int row, int start, int end, vector<glm::vec3> &pixelList,
  //   vector<GeometryObject*> &scene, vector<LightBase*> &light);

// // begin to render
// void on_pushButton_Render_clicked();
};

}
