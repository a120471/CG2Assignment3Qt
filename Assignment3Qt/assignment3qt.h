#pragma once

#include "ui_Assignment3Qt.h"
#include <QtWidgets/QMainWindow>

#include <vector>

#include "rayTracingCamera.h"
#include "geometryObject.h"
#include "lightSource.h"
#include "Utils.h"

using namespace std;

// struct for qt UI input params
struct QTInputParam
{
	int resolutionW;
	int resolutionH;
	int antiAliasingLevel;
	int imageScaleRatio;
};

class Assignment3Qt : public QMainWindow
{
	Q_OBJECT
		
public:
	Assignment3Qt(QWidget *parent = 0);
	~Assignment3Qt(){};

	void RenderImage(const QTInputParam&, vector<GeometryObject*> &scene, RayTracingCameraClass* camera, vector<LightBase*> &light);

	int RayHitTest(RayClass* ray, vector<GeometryObject*> &scene, vector<LightBase*> &light, RayHitObjectRecord &record, float lightDis = INFINITE);

	glm::vec3 calColorOnHitPoint(RayHitObjectRecord &record, vector<GeometryObject*> &scene, vector<LightBase*> &light, int level);

private:
	// scene is loaded from text, so we need to deal with the line data
	void processSceneData(vector<GeometryObject*> &scene, QString line);

	Ui::Assignment3QtClass ui;

private slots:
// choose the scene data path
void on_pushButton_Browse_clicked();
// begin to render
void on_pushButton_Render_clicked();
};
