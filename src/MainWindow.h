#pragma once

#include <vector>
#include <QMainWindow>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "Utils.h"

struct RenderParams {
  glm::uvec2 resolution;
  int multi_sampling;
  int image_scale_ratio;
};

class QLabel;
class QPushButton;
class QLineEdit;

namespace ray_tracing {

class GeometryObject;
class RayTracingCamera;
class LightBase;
struct RayHitObjectRecord;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow() = default;

  int RayHitTest(const Ray &ray,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::vector<std::shared_ptr<LightBase>> &lights,
    RayHitObjectRecord &record, float lightDis = MYINFINITE);

  glm::vec3 calColorOnHitPoint(RayHitObjectRecord &record,
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
    std::vector<glm::vec3> &pixel_list,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::vector<std::shared_ptr<LightBase>> &lights);

  QLabel *rendered_image_;
  QPushButton *scene_file_button_;
  QLineEdit *scene_file_text_;
  QPushButton *render_button_;
  QLabel *render_time_;

  std::vector<QLineEdit*> cam_pos_vec_;
  std::vector<QLineEdit*> cam_lookat_vec_;

  std::vector<QLineEdit*> resolution_vec_;
  QLineEdit *multi_sampling_;
  QLineEdit *scale_ratio_;
};

}
