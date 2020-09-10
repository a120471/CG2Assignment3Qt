#pragma once

#include <vector>
#include <QMainWindow>
#include "Utils.h"
#include "VectorWidget.h"
#include "NumberWidget.h"
#include "Type.h"

class QLabel;
class QPushButton;
class QLineEdit;

namespace ray_tracing {

class GeometryObject;
class RayTracingCamera;
class LightBase;
struct RayHitObjectRecord;

struct RenderParams {
  Vec2u resolution;
  uint32_t multi_sampling;
  uint32_t image_scale_ratio;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow() = default;

  int RayHitTest(const Ray &ray,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::vector<std::shared_ptr<LightBase>> &lights,
    RayHitObjectRecord &record, float lightDis = MYINFINITE);

  Vec3f calColorOnHitPoint(RayHitObjectRecord &record,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::vector<std::shared_ptr<LightBase>> &lights, int level);

private slots:
  void ChooseSceneFile();
  void RenderScene();

private:
  void InitUI();
  void RenderImage(const RenderParams &params,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::shared_ptr<RayTracingCamera> &camera,
    const std::vector<std::shared_ptr<LightBase>> &lights);
  void RenderPixels(const std::shared_ptr<RayTracingCamera> &camera,
    int pixel_id, uint32_t resolution_w,
    std::vector<Vec3f> &pixel_list,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::vector<std::shared_ptr<LightBase>> &lights);

  QLabel *rendered_image_;
  QPushButton *scene_file_button_;
  QLineEdit *scene_file_text_;
  QPushButton *render_button_;
  QLabel *render_time_;

  VectorWidget<uint32_t> *resolution_;
  NumberWidget<uint32_t> *multi_sampling_;
  NumberWidget<uint32_t> *scale_ratio_;

  VectorWidget<float> *cam_pos_vec_;
  VectorWidget<float> *cam_lookat_vec_;
};

}
