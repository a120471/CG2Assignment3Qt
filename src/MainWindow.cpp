#include "MainWindow.h"
#include <memory>
#include <mutex>
#include <tbb/parallel_for.h>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "LightSource.h"
#include "GeometryObject.h"
#include "RayTracingCamera.h"
#include "Utils.h"

const glm::uvec2 RESOLUTION = glm::uvec2(400u, 300u);
const uint32_t MULTI_SAMPLING = 1u;
const uint32_t IMAGE_SCALE_RATIO = 3u;
const glm::vec3 CAMERA_POS = glm::vec3(0, 1, 10);
const glm::vec3 LOOKAT = glm::vec3(0, 0, 0);
const QString SCENE_FILE_PATH = QString("");

namespace {

using namespace ray_tracing;

std::mutex mutex;

glm::vec3 QStringListToGlmVec3(const QStringList &list) {
  return glm::vec3(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
}

glm::vec4 QStringListToGlmVec4(const QStringList &list) {
  return glm::vec4(list[0].toFloat(), list[1].toFloat(), list[2].toFloat(), list[3].toFloat());
}

void ParseSceneLineData(std::vector<std::shared_ptr<GeometryObject>> &scene, const QString &line) {
  QStringList level1 = line.split(';', Qt::SkipEmptyParts);

  if (!level1.empty()) {
    QStringList level2;
    QString geometry_type = level1[0].toLower();
    if (geometry_type == "sphere") {
      level2 = level1[1].split(',');
      glm::vec3 center = QStringListToGlmVec3(level2);

      float radius = level1[2].toFloat();

      level2 = level1[3].split(',');
      glm::vec3 color = QStringListToGlmVec3(level2);

      scene.emplace_back(std::make_shared<Sphere>(center, radius, color));
    } else if (geometry_type == "plane") {
      level2 = level1[1].split(',');
      glm::vec4 ABCD = QStringListToGlmVec4(level2);

      level2 = level1[2].split(',');
      glm::vec3 color = QStringListToGlmVec3(level2);

      scene.emplace_back(std::make_shared<Plane>(ABCD, color));
    } else if (geometry_type == "model") {
      std::string filepath = level1[1].toStdString();

      level2 = level1[2].split(',');
      glm::vec3 color = QStringListToGlmVec3(level2);

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
  this->setWindowTitle("Ray Tracing");

  auto central_widget = new QWidget(this);
  this->setCentralWidget(central_widget);
  auto layout = new QHBoxLayout(central_widget);

  rendered_image_ = new QLabel("Render Result", this);
  layout->addWidget(rendered_image_);
  rendered_image_->setAlignment(Qt::AlignCenter);

  auto right_layout = new QFormLayout();
  layout->addLayout(right_layout);
  auto resolution = new QHBoxLayout();
  right_layout->addRow("Resolution", resolution);
  auto image_w = new QLineEdit(QString::number(RESOLUTION.x), this);
  auto image_h = new QLineEdit(QString::number(RESOLUTION.y), this);
  resolution->addWidget(image_w);
  resolution->addWidget(image_h);
  resolution_vec_.clear();
  resolution_vec_.emplace_back(image_w);
  resolution_vec_.emplace_back(image_h);

  multi_sampling_ = new QLineEdit(QString::number(MULTI_SAMPLING), this);
  right_layout->addRow("Multi Sampling", multi_sampling_);

  scale_ratio_ = new QLineEdit(QString::number(IMAGE_SCALE_RATIO), this);
  right_layout->addRow("Image Scale Ratio", scale_ratio_);

  auto spacer1 = new QSpacerItem(20, 20);
  right_layout->addItem(spacer1);

  auto camera_pos = new QHBoxLayout();
  right_layout->addRow("Camera Pos", camera_pos);
  auto cam_x = new QLineEdit(QString::number(CAMERA_POS.x), this);
  auto cam_y = new QLineEdit(QString::number(CAMERA_POS.y), this);
  auto cam_z = new QLineEdit(QString::number(CAMERA_POS.z), this);
  camera_pos->addWidget(cam_x);
  camera_pos->addWidget(cam_y);
  camera_pos->addWidget(cam_z);
  cam_pos_vec_.clear();
  cam_pos_vec_.emplace_back(cam_x);
  cam_pos_vec_.emplace_back(cam_y);
  cam_pos_vec_.emplace_back(cam_z);

  auto lookat = new QHBoxLayout();
  right_layout->addRow("Lookat", lookat);
  auto lookat_x = new QLineEdit(QString::number(LOOKAT.x), this);
  auto lookat_y = new QLineEdit(QString::number(LOOKAT.y), this);
  auto lookat_z = new QLineEdit(QString::number(LOOKAT.z), this);
  lookat->addWidget(lookat_x);
  lookat->addWidget(lookat_y);
  lookat->addWidget(lookat_z);
  cam_lookat_vec_.clear();
  cam_lookat_vec_.emplace_back(lookat_x);
  cam_lookat_vec_.emplace_back(lookat_y);
  cam_lookat_vec_.emplace_back(lookat_z);

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
  lights.emplace_back(std::make_shared<PointLight>(
    glm::vec3(1.3, 0, 1), glm::vec3(1, 1, 1) * 0.7f));
  lights.emplace_back(std::make_shared<PointLight>(
    glm::vec3(-1.1, 1, 0.5), glm::vec3(0.4, 0.6, 0.5) * 1.0f));
  // lights.emplace_back((LightBase*)new CubeMap("../cubeMap.hdr", 30.1f));

  // Create scene
  std::vector<std::shared_ptr<GeometryObject>> scene;
  QFile scene_file(scene_file_text_->text());
  if (!scene_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    printf("file %s not exists\n", scene_file_text_->text().toStdString().c_str());
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
  glm::vec3 cam_pos(cam_pos_vec_[0]->text().toFloat(),
    cam_pos_vec_[1]->text().toFloat(),
    cam_pos_vec_[2]->text().toFloat());
  glm::vec3 lookat(cam_lookat_vec_[0]->text().toFloat(),
    cam_lookat_vec_[1]->text().toFloat(),
    cam_lookat_vec_[2]->text().toFloat());
  RenderParams params;
  params.resolution = glm::uvec2(resolution_vec_[0]->text().toUInt(),
    resolution_vec_[1]->text().toUInt());
  params.multi_sampling = multi_sampling_->text().toInt();
  params.image_scale_ratio = scale_ratio_->text().toInt();
  auto camera = std::make_shared<RayTracingCamera>(
    cam_pos, lookat, glm::vec3(0, 1, 0), params.multi_sampling);
  camera->SetResolution(params.resolution); // pixel resolution
  camera->SetWHF(glm::vec3(8, 6, 8));  // help to set image center?
  camera->SetP(camera->getPos() + camera->getFront() * camera->GetWHF().z);

  // render image
  render_button_->setEnabled(false);
  render_button_->repaint();
  RenderImage(params, scene, camera, lights);
  render_button_->setEnabled(true);
}

void MainWindow::RenderPixels(const std::shared_ptr<RayTracingCamera> &camera,
  int pixel_id, uint32_t resolution_w,
  std::vector<glm::vec3> &pixel_list,
  const std::vector<std::shared_ptr<GeometryObject>> &scene,
  const std::vector<std::shared_ptr<LightBase>> &lights) {

  uint32_t row = pixel_id / resolution_w;
  uint32_t col = pixel_id % resolution_w;
  std::vector<Ray> rays;
  camera->GenerateRay(row, col, rays);

  // for each ray inside a pixel
  RayHitObjectRecord record;
  pixel_list[pixel_id] = glm::vec3();
  for (const auto &ray : rays) {
    // find the hit object and hit type
    int hitType = RayHitTest(ray, scene, lights, record);
    if (hitType == 1) {
      pixel_list[pixel_id] += calColorOnHitPoint(record, scene, lights, 1);
    }
    else if (hitType == 2) {
      pixel_list[pixel_id] += record.point_color;
    }
  }

  pixel_list[pixel_id] /= camera->GetRayNumEachPixel();
}

void MainWindow::RenderImage(const RenderParams &params,
    const std::vector<std::shared_ptr<GeometryObject>> &scene,
    const std::shared_ptr<RayTracingCamera> &camera,
    const std::vector<std::shared_ptr<LightBase>> &lights) {
  const clock_t begin_time = clock();

  const auto &resolution = camera->GetResolution();

  // Create a new image
  std::shared_ptr<QImage> q_image = std::make_shared<QImage>(
    resolution.x * params.image_scale_ratio,
    resolution.y * params.image_scale_ratio,
    QImage::Format_RGB888);

  // Save rendered pixels, we need to scale them later for visualization
  int pixel_num = resolution.x * resolution.y;
  std::vector<float> pixel_Rs(pixel_num);
  std::vector<float> pixel_Gs(pixel_num);
  std::vector<float> pixel_Bs(pixel_num);
  std::vector<glm::vec3> pixel_list(pixel_num);

  float brightest = 0.01f;
  tbb::parallel_for (tbb::blocked_range<int>(0, pixel_num),
    [&](tbb::blocked_range<int> r) {
    for (int i = r.begin(); i < r.end(); ++i) {
      RenderPixels(camera, i, resolution.x, pixel_list, scene, lights);

      pixel_Rs[i] = pixel_list[i][0];
      pixel_Gs[i] = pixel_list[i][1];
      pixel_Bs[i] = pixel_list[i][2];
      mutex.lock();
      brightest = std::max(brightest, pixel_Rs[i]);
      brightest = std::max(brightest, pixel_Gs[i]);
      brightest = std::max(brightest, pixel_Bs[i]);
      mutex.unlock();

      // rendered_image_->setPixmap(QPixmap::fromImage(q_image));
      // rendered_image_->repaint();
      // QCoreApplication::processEvents();
    }
  });

  // calculate the scale ratio
  int nthIdx = (int)(pixel_num * NTHIDX) - 1;
  nth_element(pixel_Rs.begin(), pixel_Rs.begin() + nthIdx, pixel_Rs.end());
  nth_element(pixel_Gs.begin(), pixel_Gs.begin() + nthIdx, pixel_Gs.end());
  nth_element(pixel_Bs.begin(), pixel_Bs.begin() + nthIdx, pixel_Bs.end());

  float maxRadiance = std::max(0.01f, pixel_Rs[nthIdx]);
  maxRadiance = std::max(maxRadiance, pixel_Gs[nthIdx]);
  maxRadiance = std::max(maxRadiance, pixel_Bs[nthIdx]);
  float scale = 255.0f / maxRadiance;

  int arrayIdx = 0;
  for (int row = 0; row < resolution.y; ++row) {
    for (int col = 0; col < resolution.x; ++col) {
      pixel_list[arrayIdx] *= scale;
      int R = std::min((int)pixel_list[arrayIdx][0], 255);
      int G = std::min((int)pixel_list[arrayIdx][1], 255);
      int B = std::min((int)pixel_list[arrayIdx][2], 255);
      for (int rowI = 0; rowI < params.image_scale_ratio; ++rowI) {
        for (int colI = 0; colI < params.image_scale_ratio; ++colI) {
          q_image->setPixel(col * params.image_scale_ratio + colI, row * params.image_scale_ratio + rowI, qRgb(R, G, B));
        }
      }

      ++arrayIdx;
    }
  }

  float timeEllapse = float(clock() - begin_time) / CLOCKS_PER_SEC;

  // display the image
  rendered_image_->setPixmap(QPixmap::fromImage(*(q_image.get())));
  render_time_->setText(QString("Time: %1s").arg(timeEllapse));
}

int MainWindow::RayHitTest(const Ray &ray,
  const std::vector<std::shared_ptr<GeometryObject>> &scene,
  const std::vector<std::shared_ptr<LightBase>> &lights,
  RayHitObjectRecord &record, float lightDis) {

  record.depth = -1;
  int hitType = 0;
  RayHitObjectRecord tmpRecord;

  for (auto object : scene) {
    object->RayIntersection(ray, tmpRecord);
    if (tmpRecord.depth - lightDis > -MYEPSILON) { // the object is further than the light source
      continue;
    }
    if (tmpRecord.depth > MYEPSILON && (record.depth > tmpRecord.depth || record.depth < MYEPSILON)) {
      record = tmpRecord;
      hitType = 1;
    }
    if (lightDis != MYINFINITE && hitType != 0) {
      return hitType;
    }
  }
  if (lightDis == MYINFINITE) {
    for (auto light : lights) {
      light->RayIntersection(ray, tmpRecord);
      if (tmpRecord.depth > MYEPSILON && (record.depth > tmpRecord.depth || record.depth < MYEPSILON)) {
        record = tmpRecord;
        hitType = 2;
      }
    }
  }

  return hitType;
}

float diffuseStrength = 0.8f;
float specularStrength = 1.0f - diffuseStrength;
float levelDegenerateRatio = 0.5f;
glm::vec3 MainWindow::calColorOnHitPoint(RayHitObjectRecord &record,
  const std::vector<std::shared_ptr<GeometryObject>> &scene,
  const std::vector<std::shared_ptr<LightBase>> &lights, int level) {
  // level starts from 1
  if (level > 3)
    return glm::vec3(0, 0, 0);

  glm::vec3 diffuse(0.0f);
  glm::vec3 specular(0.0f);

  glm::vec3 reflectionColor = glm::vec3(0, 0, 0);
  Ray reflectionRay(record.hit_point, record.r_direction);
  RayHitObjectRecord reflectionHitRecord;
  int hitType = RayHitTest(reflectionRay, scene, lights, reflectionHitRecord);
  if (hitType == 1) {
    glm::vec3 recursiveHitPointColor = calColorOnHitPoint(reflectionHitRecord, scene, lights, level + 1);
    reflectionColor = levelDegenerateRatio * std::max(dot(record.hit_normal, record.r_direction), 0.0f) * recursiveHitPointColor;
  }
  else if (hitType == 2) {
    specular += specularStrength * reflectionHitRecord.point_color;
  }

  RayHitObjectRecord lightHitRecord;
  // for each light source
  std::vector<glm::vec3> lightColorList;
  std::vector<float> lightDisList;
  std::vector<glm::vec3> lightDirList;
  for (auto light : lights) {
    lightColorList.clear();
    lightDisList.clear();
    lightDirList.clear();
    light->GetLight(record.hit_point, lightColorList, lightDisList, lightDirList);

    for (unsigned int j = 0; j < lightDirList.size(); ++j) {
      Ray lightRay(record.hit_point, lightDirList[j]);
      if (!RayHitTest(lightRay, scene, lights, lightHitRecord, lightDisList[j])) {
        float diff = std::max(dot(record.hit_normal, lightDirList[j]), 0.0f);
        diffuse += diffuseStrength * diff * lightColorList[j];
      }
    }
  }

  if (!hasHDRLighting) {
    diffuse *= 15.0f;
  } else {
    diffuse *= 1.0f;
  }

  glm::vec3 returnColor = glm::vec3(0);
  returnColor += diffuse / (float)lightDirList.size() + specular;
  //if (1 == level)
  //  returnColor = glm::vec3(0, 0, 0);

  returnColor += reflectionColor;

  returnColor *= record.point_color;

  return returnColor;
}

}
