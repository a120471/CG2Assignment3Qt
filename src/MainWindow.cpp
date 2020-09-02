#include "MainWindow.h"
#include <memory>
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

const glm::uvec2 RESOLUTION = glm::uvec2(400u, 300u);
const uint32_t MULTI_SAMPLING = 1u;
const uint32_t IMAGE_SCALE_RATIO = 3u;
const glm::vec3 CAMERA_POS = glm::vec3(0, 1, 10);
const glm::vec3 LOOKAT = glm::vec3(0, 1, 10);
const QString SCENE_FILE_PATH = QString("../../Scene.txt");

// #include <thread>
// #include <omp.h>
// #include <algorithm>
// #include <ctime>
// #include <QDebug>
// #include <QFileDialog>
// #include <QMessageBox>
// #include <QFile>

// // GLM Mathematics (glm matrices are column-major ordering)
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

// #include "Utils.h"

namespace {

using namespace ray_tracing;

void ParseSceneLineData(std::vector<GeometryObject*> &scene,
  const QString &line) {
  QStringList level1 = line.split(';', QString::SkipEmptyParts);

  if (level1.size() != 0) {
    QStringList level2;
    if (level1[0].toLower() == "sphere") {
      level2 = level1[1].split(',');
      glm::vec3 center(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

      float radius = level1[2].toFloat();

      level2 = level1[3].split(',');
      glm::vec3 color(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

      scene.emplace_back((GeometryObject*)new Sphere(center, radius, color));
    } else if (level1[0].toLower() == "plane") {
      level2 = level1[1].split(',');
      glm::vec4 ABCD(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat(), level2[3].toFloat());

      level2 = level1[2].split(',');
      glm::vec3 color(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

      scene.emplace_back((GeometryObject*)new Plane(ABCD[0], ABCD[1], ABCD[2], ABCD[3], color));
    } else if (level1[0].toLower() == "model") {
      level2 = level1[2].split(',');
      glm::vec3 color(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

      scene.emplace_back((GeometryObject*)new Model(level1[1].toStdString(), color));
    }
  }
}

}

namespace ray_tracing {

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) {
  //QObject::connect(ui.pushButton_Render, SIGNAL(clicked()),
  //  this, SLOT(on_pushButton_Render_clicked()));

  // QCoreApplication::processEvents();
  // on_pushButton_Render_clicked();
  InitUI();

  connect(scene_file_button_, &QPushButton::clicked,
    this, &MainWindow::ChooseSceneFile);
  connect(render_button_, &QPushButton::clicked,
    this, &MainWindow::RenderScene);
}

MainWindow::~MainWindow() {
}

void MainWindow::InitUI() {
  setWindowTitle("Ray Tracing");

  auto central_widget = new QWidget(this);
  setCentralWidget(central_widget);
  auto layout = new QHBoxLayout(central_widget);

  auto rendered_image = new QLabel("Render Result", this);
  layout->addWidget(rendered_image);
  rendered_image->setAlignment(Qt::AlignCenter);

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

  auto render_time = new QLabel("Time: 0s", this);
  right_layout->addRow(render_time);
}

void MainWindow::ChooseSceneFile() {
  auto dialog = std::make_shared<QFileDialog>(new QFileDialog(this));
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
  // // ALL COLORS ARE stored in RGB CHANNELS

  // Create lights
  std::vector<std::shared_ptr<LightBase>> lights;
  auto aas = new PointLight(
    glm::vec3(1.3, 0, 1), glm::vec3(1, 1, 1) * 0.7f);
  // auto light1 = std::make_shared<PointLight>(new PointLight(
  //   glm::vec3(1.3, 0, 1), glm::vec3(1, 1, 1) * 0.7f));
  // lights.emplace_back(std::move(light1));
  //light.emplace_back((LightBase*)new PointLight(glm::vec3(-1.1, 1, 0.5), glm::vec3(0.4, 0.6, 0.5) * 1.0f));
  // light.emplace_back((LightBase*)new CubeMap("../cubeMap.hdr", 30.1f));

  // Create scene
  std::vector<GeometryObject*> scene;
  QFile scene_file(scene_file_text_->text());
  if (!scene_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
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

  RayTracingCamera *camera = new RayTracingCamera(
    cam_pos, lookat, glm::vec3(0, 1, 0), params.multi_sampling);
  camera->SetResolution(params.resolution); // pixel resolution
  camera->SetWHF(glm::vec3(8, 6, 8));  // help to set image center?
  camera->SetP(camera->getPos() + camera->getFront() * camera->GetWHF().z);

  // // render image
  // this->ui.pushButton_Render->setEnabled(false);
  // this->ui.pushButton_Render->repaint();
  // this->RenderImage(params, scene, camera, light);
  // this->ui.pushButton_Render->setEnabled(true);

  // // delete camera
  // safe_delete(camera);
  // // delete scene
  // for (vector<GeometryObject*>::iterator i = scene.begin(); i != scene.end(); i++)
  //   safe_delete(*i);
  // // delete light
  // for (vector<LightBase*>::iterator i = light.begin(); i != light.end(); i++)
  //   safe_delete(*i);
}

// void MainWindow::RenderPixels(RayTracingCameraClass* camera, int row, int start, int end, vector<glm::vec3> &pixelList,
//   vector<GeometryObject*> &scene, vector<LightBase*> &light)
// {
//   vector<RayClass*> rayList;
//   RayHitObjectRecord curRayRecord;
//   int arrayIdx = row * camera->getW() + start;

//   for (int col = start; col < end; col++)
//   {
//     camera->GenerateRay(row, col, rayList);
//     // for each ray inside a pixel
//     pixelList[arrayIdx] = glm::vec3();
//     for (vector<RayClass*>::iterator i = rayList.begin(); i != rayList.end(); i++)
//     {
//       // find the hit object and hit type
//       int hitType = RayHitTest(*i, scene, light, curRayRecord);
//       if (hitType == 1)
//         pixelList[arrayIdx] += calColorOnHitPoint(curRayRecord, scene, light, 1);
//       else if (hitType == 2)
//         pixelList[arrayIdx] += curRayRecord.pointColor;

//       safe_delete(*i);
//     }
//     rayList.clear();

//     pixelList[arrayIdx] /= camera->getRayNumEachPixel();

//     arrayIdx++;
//   }
// }

// void MainWindow::RenderImage(const QTInputParam &qtIP, vector<GeometryObject*> &scene, RayTracingCameraClass* camera, vector<LightBase*> &light)
// {
//   const clock_t begin_time = clock();

//   // create a new image
//   QImage *qImage = new QImage(camera->getW() * qtIP.imageScaleRatio, camera->getH() * qtIP.imageScaleRatio, QImage::Format_RGB888);

//   // save all rendered pixels, we need to scale them later for visualization
//   int pixelNum = camera->getH() * camera->getW();
//   vector<float> pixelListR;
//   vector<float> pixelListG;
//   vector<float> pixelListB;
//   vector<glm::vec3> pixelList;
//   pixelListR.resize(pixelNum);
//   pixelListG.resize(pixelNum);
//   pixelListB.resize(pixelNum);
//   pixelList.resize(pixelNum);

//   //#pragma omp parallel for
//   float localMax = 0.01f;
//   for (int row = 0; row < camera->getH(); row++)
//   {
//     // use multi threads, when each task is not heavy, multi thread will even slow down the process
//     std::thread processPixel[MYTHREADNUM];

//     int start = 0, end = camera->getW() / MYTHREADNUM;
//     for (int i = 0; i < MYTHREADNUM; i++)
//     {
//       processPixel[i] = std::thread(&MainWindow::RenderPixels, this, camera, row, start, end, std::ref(pixelList), scene, light);
//       start = end;
//       end += camera->getW() / MYTHREADNUM;
//       if (i == MYTHREADNUM - 1)
//         end = camera->getW() - 1;
//     }

//     // Join the threads
//     for (int i = 0; i < MYTHREADNUM; i++)
//       processPixel[i].join();

//     int arrayIdx = row * camera->getW();
//     for(int col = 0; col < camera->getW(); col++)
//     {
//       pixelListR[arrayIdx] = pixelList[arrayIdx][0];
//       pixelListG[arrayIdx] = pixelList[arrayIdx][1];
//       pixelListB[arrayIdx] = pixelList[arrayIdx][2];
//       localMax = glm::max(localMax, pixelListR[arrayIdx]);
//       localMax = glm::max(localMax, pixelListG[arrayIdx]);
//       localMax = glm::max(localMax, pixelListB[arrayIdx]);

//       float localScale = 255.0f / localMax;
//       int R = min((int)(pixelList[arrayIdx][0] * localScale), 255);
//       int G = min((int)(pixelList[arrayIdx][1] * localScale), 255);
//       int B = min((int)(pixelList[arrayIdx][2] * localScale), 255);
//       for (int rowI = 0; rowI < qtIP.imageScaleRatio; rowI++)
//         for (int colI = 0; colI < qtIP.imageScaleRatio; colI++)
//           qImage->setPixel(col * qtIP.imageScaleRatio + colI, row * qtIP.imageScaleRatio + rowI, qRgb(R, G, B));

//       arrayIdx++;
//     }

//     ui.label_Image->setPixmap(QPixmap::fromImage(*qImage));
//     this->ui.label_Image->repaint();
//     QCoreApplication::processEvents();
//   }

//   // calculate the scale ratio
//   int nthIdx = pixelNum * NTHIDX - 1;
//   nth_element(pixelListR.begin(), pixelListR.begin() + nthIdx, pixelListR.end());
//   nth_element(pixelListG.begin(), pixelListG.begin() + nthIdx, pixelListG.end());
//   nth_element(pixelListB.begin(), pixelListB.begin() + nthIdx, pixelListB.end());

//   float maxRadiance = glm::max(0.01f, pixelListR[nthIdx]);
//   maxRadiance = glm::max(maxRadiance, pixelListG[nthIdx]);
//   maxRadiance = glm::max(maxRadiance, pixelListB[nthIdx]);
//   float scale = 255.0f / maxRadiance;

//   int arrayIdx = 0;
//   for (int row = 0; row < camera->getH(); row++)
//   {
//     for (int col = 0; col < camera->getW(); col++)
//     {
//       pixelList[arrayIdx] *= scale;
//       int R = min((int)pixelList[arrayIdx][0], 255);
//       int G = min((int)pixelList[arrayIdx][1], 255);
//       int B = min((int)pixelList[arrayIdx][2], 255);
//       for (int rowI = 0; rowI < qtIP.imageScaleRatio; rowI++)
//         for (int colI = 0; colI < qtIP.imageScaleRatio; colI++)
//           qImage->setPixel(col * qtIP.imageScaleRatio + colI, row * qtIP.imageScaleRatio + rowI, qRgb(R, G, B));

//       arrayIdx++;
//     }
//   }

//   float timeEllapse = float(clock() - begin_time) / CLOCKS_PER_SEC;

//   // display the image
//   ui.label_Image->setPixmap(QPixmap::fromImage(*qImage));
//   ui.label_TValue->setText(QString().sprintf("Time: %.2fs", timeEllapse));
//   safe_delete(qImage);
// }

// int MainWindow::RayHitTest(RayClass* ray, vector<GeometryObject*> &scene, vector<LightBase*> &light, RayHitObjectRecord &record, float lightDis)
// {
//   record.depth = -1;
//   int hitType = 0;
//   RayHitObjectRecord tmpRecord;

//   for (vector<GeometryObject*>::iterator j = scene.begin(); j != scene.end(); j++)
//   {
//     (*j)->RayIntersection(ray, tmpRecord);
//     if (tmpRecord.depth - lightDis > -MYEPSILON) // the object is further than the light source
//       continue;
//     if (tmpRecord.depth > MYEPSILON && (record.depth > tmpRecord.depth || record.depth < MYEPSILON))
//     {
//       record = tmpRecord;
//       hitType = 1;
//     }
//     if (lightDis != MYINFINITE && hitType != 0)
//       return hitType;
//   }
//   if (lightDis == MYINFINITE)
//   {
//     for (vector<LightBase*>::iterator j = light.begin(); j != light.end(); j++)
//     {
//       (*j)->RayIntersection(ray, tmpRecord);
//       if (tmpRecord.depth > MYEPSILON && (record.depth > tmpRecord.depth || record.depth < MYEPSILON))
//       {
//         record = tmpRecord;
//         hitType = 2;
//       }
//     }
//   }

//   return hitType;
// }

// float diffuseStrength = 0.8f;
// float specularStrength = 1.0f - diffuseStrength;
// float levelDegenerateRatio = 0.5f;
// glm::vec3 MainWindow::calColorOnHitPoint(RayHitObjectRecord &record, vector<GeometryObject*> &scene, vector<LightBase*> &light, int level)
// {
//   // level starts from 1
//   if (level > 3)
//     return glm::vec3(0, 0, 0);

//   glm::vec3 diffuse(0.0f);
//   glm::vec3 specular(0.0f);

//   glm::vec3 reflectionColor = glm::vec3(0, 0, 0);
//   RayClass* reflectionRay = new RayClass(record.hitPoint, record.rDirection);
//   RayHitObjectRecord reflectionHitRecord;
//   int hitType = RayHitTest(reflectionRay, scene, light, reflectionHitRecord);
//   if (hitType == 1)
//   {
//     glm::vec3 recursiveHitPointColor = calColorOnHitPoint(reflectionHitRecord, scene, light, level + 1);
//     reflectionColor = levelDegenerateRatio * max(dot(record.hitNormal, record.rDirection), 0.0f) * recursiveHitPointColor;
//   }
//   else if (hitType == 2)
//   {
//     specular += specularStrength * reflectionHitRecord.pointColor;
//   }
//   safe_delete(reflectionRay);

//   RayHitObjectRecord lightHitRecord;
//   // for each light source
//   std::vector<glm::vec3> lightColorList;
//   std::vector<float> lightDisList;
//   std::vector<glm::vec3> lightDirList;
//   for (vector<LightBase*>::iterator i = light.begin(); i != light.end(); i++)
//   {
//     lightColorList.clear();
//     lightDisList.clear();
//     lightDirList.clear();
//     (*i)->GetLight(record.hitPoint, lightColorList, lightDisList, lightDirList);

//     for (unsigned int j = 0; j < lightDirList.size(); j++)
//     {
//       RayClass* lightRay = new RayClass(record.hitPoint, lightDirList[j]);
//       if (!RayHitTest(lightRay, scene, light, lightHitRecord, lightDisList[j]))
//       {
//         float diff = max(dot(record.hitNormal, lightDirList[j]), 0.0f);
//         diffuse += diffuseStrength * diff * lightColorList[j];
//       }

//       safe_delete(lightRay);
//     }
//   }

//   if (!hasHDRLighting)
//     diffuse *= 15.0f;
//   else
//     diffuse *= 1.0f;

//   glm::vec3 returnColor = glm::vec3(0);
//   returnColor += diffuse / (float)lightDirList.size() + specular;
//   //if (1 == level)
//   //  returnColor = glm::vec3(0, 0, 0);

//   returnColor += reflectionColor;

//   returnColor *= record.pointColor;

//   return returnColor;
// }

}
