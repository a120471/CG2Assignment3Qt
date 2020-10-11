#include "MainWindow.h"
#include <memory>
#include <mutex>
#include <chrono>
#include <tbb/parallel_for.h>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "LightSource.h"
#include "GeometryObject.h"
#include "RayTracingCamera.h"
#include "Utils.h"

const std::vector<uint32_t> RESOLUTION{400u, 300u};
const uint32_t IMAGE_SCALE_RATIO = 3u;
const std::vector<float> CAMERA_POS{0.f, 1.f, 10.f};
const std::vector<float> LOOKAT{0.f, 0.f, 0.f};
const QString SCENE_FILE_PATH = QString("/home/dushuai/GitProject/CG2Assignment3Qt/resources/scene.txt");
const std::string CUBE_MAP_PATH = "/home/dushuai/GitProject/CG2Assignment3Qt/resources/cubemap.hdr";

using namespace ray_tracing;

namespace {

std::mutex mutex;

void CreateLights(std::vector<std::shared_ptr<LightBase>> &lights) {
  lights.emplace_back(std::make_shared<PointLight>(
    Vec3f(1.3f, 0.f, 1.f), Vec3f::Constant(0.7f)));
  lights.emplace_back(std::make_shared<PointLight>(
    Vec3f(-1.1f, 1.f, 0.5f), Vec3f(0.4f, 0.6f, 0.5f)));
  lights.emplace_back(std::make_shared<CubeMap>(CUBE_MAP_PATH, 30.1f));
}

Vec3f QStringListToVec3f(const QStringList &list) {
  return Vec3f(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
}

Vec4f QStringListToVec4f(const QStringList &list) {
  return Vec4f(list[0].toFloat(), list[1].toFloat(),
    list[2].toFloat(), list[3].toFloat());
}

void ParseSceneLineData(std::vector<std::shared_ptr<GeometryObject>> &scene,
  const QString &line) {
  QStringList parts = line.split(';', Qt::SkipEmptyParts);

  if (!parts.empty()) {
    QString geometry_type = parts[0].toLower();
    if (geometry_type == "sphere") {
      Vec3f center = QStringListToVec3f(parts[1].split(','));
      float radius = parts[2].toFloat();
      Vec3f color = QStringListToVec3f(parts[3].split(','));
      scene.emplace_back(std::make_shared<Sphere>(center, radius, color));
    } else if (geometry_type == "plane") {
      Vec4f ABCD = QStringListToVec4f(parts[1].split(','));
      Vec3f color = QStringListToVec3f(parts[2].split(','));
      scene.emplace_back(std::make_shared<Plane>(ABCD, color));
    } else if (geometry_type == "model") {
      std::string filepath = parts[1].toStdString();
      Vec3f color = QStringListToVec3f(parts[2].split(','));
      scene.emplace_back(std::make_shared<Model>(filepath, color));
    }
  }
}

}

namespace ray_tracing {

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) {
  InitUI();
  connect(scene_file_button_, &QPushButton::clicked,
    this, &MainWindow::ChooseSceneFile);
  connect(render_button_, &QPushButton::clicked,
    this, &MainWindow::RenderScene);
}

void MainWindow::InitUI() {
  this->setWindowTitle("Ray Tracing Program");

  auto central_widget = new QWidget(this);
  this->setCentralWidget(central_widget);
  auto layout = new QHBoxLayout(central_widget);

  rendered_image_ = new QLabel("Render Result", this);
  layout->addWidget(rendered_image_);
  rendered_image_->setAlignment(Qt::AlignCenter);

  auto right_layout = new QFormLayout();
  layout->addLayout(right_layout);
  resolution_ = new VectorWidget(RESOLUTION, this);
  right_layout->addRow("Resolution", resolution_);

  scale_ratio_ = new NumberWidget(IMAGE_SCALE_RATIO, this);
  right_layout->addRow("Image Scale Ratio", scale_ratio_);

  auto spacer1 = new QSpacerItem(20, 20);
  right_layout->addItem(spacer1);

  cam_pos_vec_ = new VectorWidget(CAMERA_POS, this);
  right_layout->addRow("Camera Pos", cam_pos_vec_);

  cam_lookat_vec_ = new VectorWidget(LOOKAT, this);
  right_layout->addRow("Lookat", cam_lookat_vec_);

  auto spacer2 = new QSpacerItem(20, 20);
  right_layout->addItem(spacer2);

  scene_file_button_ = new QPushButton("Browse...", this);
  right_layout->addRow("Scene Data", scene_file_button_);
  scene_file_text_ = new QLineEdit(SCENE_FILE_PATH, this);
  right_layout->addRow(scene_file_text_);
  scene_file_text_->setReadOnly(true);

  auto spacer3 = new QSpacerItem(20, 20);
  right_layout->addItem(spacer3);

  render_button_ = new QPushButton("Render", this);
  right_layout->addRow(render_button_);

  render_time_ = new QLabel("Time: 0s", this);
  right_layout->addRow(render_time_);
}

void MainWindow::ChooseSceneFile() {
  auto dialog = std::make_shared<QFileDialog>(this);
  dialog->setWindowTitle("Select the scene file");
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setNameFilter(QString("Text (*.txt)"));
  if (dialog->exec() == QDialog::Accepted) {
    QString path = dialog->selectedFiles()[0];

    QByteArray ba = path.toLatin1();
    scene_file_text_->setText(ba.data());
  }
}

void MainWindow::RenderScene() {
  // Create lights
  std::vector<std::shared_ptr<LightBase>> lights;
  CreateLights(lights);

  // Create scene
  std::vector<std::shared_ptr<GeometryObject>> scene;
  QFile scene_file(scene_file_text_->text());
  if (!scene_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << QString("file %1 not exists").arg(scene_file_text_->text());
    return;
  }
  QTextStream in(&scene_file);
  while (!in.atEnd()) {
    QString line = in.readLine();
    line = line.remove(' ');
    ParseSceneLineData(scene, line);
  }
  scene_file.close();

  // Create camera
  Vec3f cam_pos = cam_pos_vec_->GetValue();
  Vec3f lookat = cam_lookat_vec_->GetValue();
  RenderParams params;
  params.resolution = resolution_->GetValue();
  params.image_scale_ratio = scale_ratio_->GetValue();
  auto camera = std::make_shared<RayTracingCamera>(
    cam_pos, lookat, Vec3f(0.f, 1.f, 0.f));
  camera->SetK(params.resolution, 90.f);

  // Render image
  render_button_->setEnabled(false);
  render_button_->repaint();
  RenderImage(params, scene, camera, lights);
  render_button_->setEnabled(true);
}

void MainWindow::RenderImage(const RenderParams &params,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::shared_ptr<RayTracingCamera> &camera,
    const std::vector<std::shared_ptr<LightBase>> &lights) {
  auto s_time = std::chrono::system_clock::now();

  const auto &resolution = camera->GetResolution();

  // Create image
  std::shared_ptr<QImage> q_image = std::make_shared<QImage>(
    resolution(0) * params.image_scale_ratio,
    resolution(1) * params.image_scale_ratio,
    QImage::Format_RGB888);

  // Rendered pixels
  int pixel_num = resolution(0) * resolution(1);
  std::vector<Vec3f> rgbs(pixel_num);

  tbb::parallel_for (tbb::blocked_range<int>(0, pixel_num),
    [&](tbb::blocked_range<int> r) {
    for (int i = r.begin(); i < r.end(); ++i) {
      RenderPixels(camera, i, resolution(0), rgbs, scene, lights);
    }
  });

  // Calculate factor to tune rgb value
  int nth_idx = (int)(pixel_num * NTHIDX) - 1;
  auto rgbs_copy = rgbs;
  nth_element(rgbs_copy.begin(), rgbs_copy.begin() + nth_idx, rgbs_copy.end(),
    [](const Vec3f &a, const Vec3f &b) {
      return a(0) < b(0);
    });
  float max_radiance = std::max(0.01f, rgbs_copy[nth_idx](0));
  nth_element(rgbs_copy.begin(), rgbs_copy.begin() + nth_idx, rgbs_copy.end(),
    [](const Vec3f &a, const Vec3f &b) {
      return a(1) < b(1);
    });
  max_radiance = std::max(max_radiance, rgbs_copy[nth_idx](1));
  nth_element(rgbs_copy.begin(), rgbs_copy.begin() + nth_idx, rgbs_copy.end(),
    [](const Vec3f &a, const Vec3f &b) {
      return a(2) < b(2);
    });
  max_radiance = std::max(max_radiance, rgbs_copy[nth_idx](2));
  float scale = 255.0f / max_radiance;

  int i = 0;
  for (int row = 0; row < resolution(1); ++row) {
    for (int col = 0; col < resolution(0); ++col) {
      rgbs[i] *= scale;
      int R = (int)std::min(rgbs[i](0), 255.f);
      int G = (int)std::min(rgbs[i](1), 255.f);
      int B = (int)std::min(rgbs[i](2), 255.f);
      for (int rowI = 0; rowI < params.image_scale_ratio; ++rowI) {
        for (int colI = 0; colI < params.image_scale_ratio; ++colI) {
          q_image->setPixel(col * params.image_scale_ratio + colI,
            row * params.image_scale_ratio + rowI, qRgb(R, G, B));
        }
      }

      ++i;
    }
  }

  // Display the image
  auto e_time = std::chrono::system_clock::now();
  auto delta_time = std::chrono::duration<float>(e_time - s_time).count();
  rendered_image_->setPixmap(QPixmap::fromImage(*q_image));
  render_time_->setText(QString("Time: %1s").arg(delta_time));
}

void MainWindow::RenderPixels(const std::shared_ptr<RayTracingCamera> &camera,
  int pixel_id, uint32_t resolution_w,
  std::vector<Vec3f> &rgbs,
  const std::vector<std::shared_ptr<GeometryObject>> &scene,
  const std::vector<std::shared_ptr<LightBase>> &lights) {

  uint32_t row = pixel_id / resolution_w;
  uint32_t col = pixel_id % resolution_w;
  Ray ray = camera->GenerateRay(row, col);

  RayHitRecord record;

  // Find the hit object and hit type
  int hitType = RayHitTest(ray, scene, lights, record);
  if (hitType == 1) {
    rgbs[pixel_id] = calColorOnHitPoint(record, scene, lights, 1);
  } else if (hitType == 2) {
    rgbs[pixel_id] = record.point_color;
  } else {
    rgbs[pixel_id] = Vec3f::Zero();
  }
}

int MainWindow::RayHitTest(const Ray &ray,
  const std::vector<std::shared_ptr<GeometryObject>> &scene,
  const std::vector<std::shared_ptr<LightBase>> &lights,
  RayHitRecord &record, float lightDis) {

  record.depth = -1;
  int hitType = 0;
  RayHitRecord tmp_record;

  for (auto object : scene) {
    tmp_record = object->RayIntersection(ray);
    if (tmp_record.depth - lightDis > -MYEPSILON) { // Object is further than the light source
      continue;
    }
    if (tmp_record.depth > MYEPSILON && (record.depth > tmp_record.depth || record.depth < MYEPSILON)) {
      record = tmp_record;
      hitType = 1;
    }
    if (lightDis != MYINFINITE && hitType != 0) {
      return hitType;
    }
  }
  // if (lightDis == MYINFINITE) {
  //   for (auto light : lights) {
  //     light->RayIntersection(ray, tmp_record);
  //     if (tmp_record.depth > MYEPSILON && (record.depth > tmp_record.depth || record.depth < MYEPSILON)) {
  //       record = tmp_record;
  //       hitType = 2;
  //     }
  //   }
  // }

  return hitType;
}

float diffuseStrength = 0.8f;
float specularStrength = 1.0f - diffuseStrength;
float levelDegenerateRatio = 0.5f;
Vec3f MainWindow::calColorOnHitPoint(RayHitRecord &record,
  const std::vector<std::shared_ptr<GeometryObject>> &scene,
  const std::vector<std::shared_ptr<LightBase>> &lights, int level) {
  // Level starts from 1
  if (level > 3)
    return Vec3f::Zero();

  Vec3f diffuse = Vec3f::Zero();
  Vec3f specular = Vec3f::Zero();

  Vec3f reflectionColor = Vec3f::Zero();
  Ray reflectionRay(record.hit_point, record.r_direction);
  RayHitRecord reflectionHitRecord;
  int hitType = RayHitTest(reflectionRay, scene, lights, reflectionHitRecord);
  if (hitType == 1) {
    Vec3f recursiveHitPointColor = calColorOnHitPoint(reflectionHitRecord, scene, lights, level + 1);
    reflectionColor = levelDegenerateRatio * std::max(record.hit_normal.dot(record.r_direction), 0.0f) * recursiveHitPointColor;
  } else if (hitType == 2) {
    specular += specularStrength * reflectionHitRecord.point_color;
  }

  RayHitRecord lightHitRecord;
  // For each light source
  std::vector<LightInfoToPoint> infos;
  for (auto light : lights) {
    infos.clear();
    light->AppendToLightInfo(record.hit_point, infos);

    for (const auto &info : infos) {
      Ray lightRay(record.hit_point, info.direction);
      if (!RayHitTest(lightRay, scene, lights, lightHitRecord, info.distance)) {
        float diff = std::max(record.hit_normal.dot(info.direction), 0.0f);
        diffuse += diffuseStrength * diff * info.color;
      }
    }
  }

  if (!hasHDRLighting) {
    diffuse *= 15.0f;
  } else {
    diffuse *= 1.0f;
  }

  Vec3f returnColor = Vec3f::Zero();
  returnColor += diffuse / (float)infos.size() + specular;
  //if (1 == level)
  //  returnColor = Vec3f::Zero();

  returnColor += reflectionColor;

  returnColor.cwiseProduct(record.point_color);

  return returnColor;
}

}
