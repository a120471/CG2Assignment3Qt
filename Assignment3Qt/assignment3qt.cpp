#include "assignment3qt.h"

#include <omp.h>
#include <algorithm>
#include <ctime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>

// GLM Mathematics (glm matrices are column-major ordering)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

Assignment3Qt::Assignment3Qt(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	//QObject::connect(ui.pushButton_Render, SIGNAL(clicked()),
	//	this, SLOT(on_pushButton_Render_clicked()));

	//on_pushButton_Render_clicked();
}

// choose the scene data path
void Assignment3Qt::on_pushButton_Browse_clicked()
{
	QFileDialog *fileDialog = new QFileDialog(this);
	fileDialog->setWindowTitle("select the sceneData file");
	fileDialog->setDirectory("");
	fileDialog->setNameFilter(QString("Text (*.txt)"));
	if (fileDialog->exec() == QDialog::Accepted)
	{
		QString path = fileDialog->selectedFiles()[0];

		QByteArray ba = path.toLatin1();
		ui.sceneDataPath->setText(ba.data());
	}
	safe_delete(fileDialog);
}

// begin to render
void Assignment3Qt::on_pushButton_Render_clicked()
{
	// ALL COLORS ARE stored in BGR CHANNELS

	// create light
	vector<LightBase*> light;
	//light.push_back((LightBase*)new PointLight(glm::vec3(1.3, -1.3, -1.3), glm::vec3(1, 1, 1) * 0.7f));
	//light.push_back((LightBase*)new PointLight(glm::vec3(0.8, -0.5, 3.3), glm::vec3(0.5, 0.6, 0.4) * 1.0f));
	light.push_back((LightBase*)new CubeMap("../cubeMap.hdr", 2.1f));

	// create scene from file
	vector<GeometryObject*> scene;
	QString sceneDataPath = ui.sceneDataPath->text();
	QFile sceneDataFile(sceneDataPath);
	if (!sceneDataFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream in(&sceneDataFile);
	while (!in.atEnd())
	{
		QString line = in.readLine();
		line = line.remove(' ');
		processSceneData(scene, line);
	}
	sceneDataFile.close();
	
	// read camera param
	QStringList cameraPosList = ui.CameraPos->text().split(',');
	glm::vec3 cameraPos = glm::vec3(cameraPosList[0].toFloat(), cameraPosList[1].toFloat(), cameraPosList[2].toFloat());
	QStringList cameraLookatList = ui.CameraLookAt->text().split(',');
	glm::vec3 cameraLookat = glm::vec3(cameraLookatList[0].toFloat(), cameraLookatList[1].toFloat(), cameraLookatList[2].toFloat());
	QTInputParam qtIP;
	qtIP.resolutionW = ui.resolutionW->text().toInt();
	qtIP.resolutionH = ui.resolutionH->text().toInt();
	qtIP.antiAliasingLevel = ui.antiAliasing->text().toInt();
	qtIP.imageScaleRatio = ui.imageScaleRatio->text().toInt();
	// create camera
	RayTracingCameraClass* camera = new RayTracingCameraClass(cameraPos, cameraLookat, glm::vec3(0, 1, 0), qtIP.antiAliasingLevel);
	camera->setW(qtIP.resolutionW); // pixel width resolution
	camera->setH(qtIP.resolutionH); // pixel height resolution
	camera->setFL(8);  // help to set image center?
	camera->setIW(8);
	camera->setIH(6);
	camera->setP(camera->getPos() + camera->getFront() * camera->getFL());

	// render image
	this->ui.pushButton_Render->setEnabled(false);
	this->repaint();
	this->RenderImage(qtIP, scene, camera, light);
	this->ui.pushButton_Render->setEnabled(true);

	// delete camera
	safe_delete(camera);
	// delete scene
	for (vector<GeometryObject*>::iterator i = scene.begin(); i != scene.end(); i++)
		safe_delete(*i);
	// delete light
	for (vector<LightBase*>::iterator i = light.begin(); i != light.end(); i++)
		safe_delete(*i);
}

void Assignment3Qt::RenderImage(const QTInputParam &qtIP, vector<GeometryObject*> &scene, RayTracingCameraClass* camera, vector<LightBase*> &light)
{
	const clock_t begin_time = clock();

	// create a new image
	QImage *qImage = new QImage(camera->getW() * qtIP.imageScaleRatio, camera->getH() * qtIP.imageScaleRatio, QImage::Format_RGB888);
	
	// save all rendered pixels, we need to scale them later for visualization
	int pixelNum = camera->getH() * camera->getW();
	vector<float> pixelListR;
	vector<float> pixelListG;
	vector<float> pixelListB;
	vector<glm::vec3> pixelList; // B G R
	pixelListR.resize(pixelNum);
	pixelListG.resize(pixelNum);
	pixelListB.resize(pixelNum);
	pixelList.resize(pixelNum);

	vector<RayClass*> rayList;
	RayHitObjectRecord curRayRecord;
	//#pragma omp parallel for
	int arrayIdx = 0;
	for (int row = 0; row < camera->getH(); row++)
	{
		for (int col = 0; col < camera->getW(); col++)
		{
			camera->GenerateRay(row, col, rayList);
			// for each ray inside a pixel
			pixelList[arrayIdx] = glm::vec3();
			for (vector<RayClass*>::iterator i = rayList.begin(); i != rayList.end(); i++)
			{
				// find the hit object and hit type
				int hitType = RayHitTest(*i, scene, light, curRayRecord);
				if (hitType == 1)
				{
					//pixelList[arrayIdx] += calColorOnHitPoint(curRayRecord, scene, light, 1);
				}
				else if (hitType == 2)
					pixelList[arrayIdx] += curRayRecord.pointColor;

				safe_delete(*i);
			}
			rayList.clear();
			
			pixelList[arrayIdx] /= camera->getRayNumEachPixel();
			pixelListR[arrayIdx] = pixelList[arrayIdx][2];
			pixelListG[arrayIdx] = pixelList[arrayIdx][1];
			pixelListB[arrayIdx] = pixelList[arrayIdx][0];
			arrayIdx++;
		}
	}

	// calculate the scale ratio
	int nthIdx = pixelNum * NTHIDX;
	nth_element(pixelListR.begin(), pixelListR.begin() + nthIdx, pixelListR.end());
	nth_element(pixelListG.begin(), pixelListG.begin() + nthIdx, pixelListG.end());
	nth_element(pixelListB.begin(), pixelListB.begin() + nthIdx, pixelListB.end());

	float maxRadiance = glm::max(1.0f, pixelListR[nthIdx]);
	maxRadiance = glm::max(maxRadiance, pixelListG[nthIdx]);
	maxRadiance = glm::max(maxRadiance, pixelListB[nthIdx]);
	float scale = 255.0f / maxRadiance;

	arrayIdx = 0;
	for (int row = 0; row < camera->getH(); row++)
	{
		for (int col = 0; col < camera->getW(); col++)
		{
			//pixelList[arrayIdx] *= scale;
			int B = min((int)pixelList[arrayIdx][0], 255);
			int G = min((int)pixelList[arrayIdx][1], 255);
			int R = min((int)pixelList[arrayIdx][2], 255);
			for (int rowI = 0; rowI < qtIP.imageScaleRatio; rowI++)
				for (int colI = 0; colI < qtIP.imageScaleRatio; colI++)
					qImage->setPixel(col * qtIP.imageScaleRatio + colI, row * qtIP.imageScaleRatio + rowI, qRgb(R, G, B));

			arrayIdx++;
		}
	}

	float timeEllapse = float(clock() - begin_time) / CLOCKS_PER_SEC;

	// display the image
	ui.label_Image->setPixmap(QPixmap::fromImage(*qImage));
	ui.label_TValue->setText(QString().sprintf("Time: %.2fs", timeEllapse));
	safe_delete(qImage);
}

int Assignment3Qt::RayHitTest(RayClass* ray, vector<GeometryObject*> &scene, vector<LightBase*> &light, RayHitObjectRecord &record, float lightDis)
{
	record.depth = -1;
	int hitType = 0;
	RayHitObjectRecord tmpRecord;

	for (vector<GeometryObject*>::iterator j = scene.begin(); j != scene.end(); j++)
	{
		(*j)->RayIntersection(ray, tmpRecord);
		if (tmpRecord.depth - lightDis > -EPSILON) // the object is further than the light source
			continue;
		if (tmpRecord.depth > EPSILON && (record.depth > tmpRecord.depth || record.depth < EPSILON))
		{
			record = tmpRecord;
			hitType = 1;
		}
		if (lightDis != INFINITE && hitType != 0)
			return hitType;
	}
	if (lightDis == INFINITE)
	{
		for (vector<LightBase*>::iterator j = light.begin(); j != light.end(); j++)
		{
			(*j)->RayIntersection(ray, tmpRecord);
			if (tmpRecord.depth > EPSILON && (record.depth > tmpRecord.depth || record.depth < EPSILON))
			{
				record = tmpRecord;
				hitType = 2;
			}
			if (lightDis != INFINITE && hitType != 0)
				return hitType;
		}
	}

	return hitType;
}

float ambientStrength = 0.0f;
float diffuseStrength = 0.3f;
float specularStrength = 0.3f;
float levelDegenerateRatio = 0.3f;
glm::vec3 Assignment3Qt::calColorOnHitPoint(RayHitObjectRecord &record, vector<GeometryObject*> &scene, vector<LightBase*> &light, int level)
{
	// level starts from 1
	if (level > 1)
		return glm::vec3(0, 0, 0);
	
	glm::vec3 reflectionColor = glm::vec3(0, 0, 0);
	RayClass* reflectionRay = new RayClass(record.hitPoint, record.rDirection);
	RayHitObjectRecord reflectionHitRecord;
	if (RayHitTest(reflectionRay, scene, light, reflectionHitRecord))
		reflectionColor = levelDegenerateRatio * max(dot(record.hitNormal, record.rDirection), 0.0f) * calColorOnHitPoint(reflectionHitRecord, scene, light, level + 1);
	safe_delete(reflectionRay);

	glm::vec3 returnColor = glm::vec3(0, 0, 0);
	RayHitObjectRecord lightHitRecord;
	// for each light source
	std::vector<glm::vec3> lightColorList;
	std::vector<float> lightDisList;
	std::vector<glm::vec3> lightDirList;
	for (vector<LightBase*>::iterator i = light.begin(); i != light.end(); i++)
	{
		lightColorList.clear();
		lightDisList.clear();
		lightDirList.clear();
		(*i)->GetLight(record.hitPoint, lightColorList, lightDisList, lightDirList);

		glm::vec3 ambientColor(0.0f);
		glm::vec3 diffuse(0.0f);
		glm::vec3 specular(0.0f);
		for (unsigned int j = 0; j < lightDirList.size(); j++)
		{
			ambientColor += ambientStrength * lightColorList[j];

			RayClass* lightRay = new RayClass(record.hitPoint, lightDirList[j]);
			if (!RayHitTest(lightRay, scene, light, lightHitRecord, lightDisList[j]))
			{
				float diff = max(dot(record.hitNormal, lightDirList[j]), 0.0f);
				diffuse += diffuseStrength * diff * lightColorList[j];

				float spec = pow(max(dot(record.rDirection, lightDirList[j]), 0.0f), 512);
				specular += specularStrength * spec * lightColorList[j];
			}

			safe_delete(lightRay);
		}
		returnColor += ambientColor + diffuse / (float)lightDirList.size() + specular;
	}
	//if (1 == level)
	//	returnColor = glm::vec3(0, 0, 0);

	returnColor += reflectionColor;

	returnColor *= record.pointColor;
	return returnColor;
}

void Assignment3Qt::processSceneData(vector<GeometryObject*> &scene, QString line)
{
	QStringList level1 = line.split(';', QString::SkipEmptyParts);

	if (level1.length() != 0)
	{
		QStringList level2;
		if ("Sphere" == level1[0] || "sphere" == level1[0])
		{
			level2 = level1[1].split(',');
			glm::vec3 center = glm::vec3(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

			float radius = level1[2].toFloat();

			level2 = level1[3].split(',');
			glm::vec3 color = glm::vec3(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

			scene.push_back((GeometryObject*)new Sphere(center, radius, color));
		}
		else if ("Plane" == level1[0] || "plane" == level1[0])
		{
			level2 = level1[1].split(',');
			glm::vec4 ABCD = glm::vec4(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat(), level2[3].toFloat());

			level2 = level1[2].split(',');
			glm::vec3 color = glm::vec3(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

			scene.push_back((GeometryObject*)new Plane(ABCD[0], ABCD[1], ABCD[2], ABCD[3], color));
		}
		else if ("Model" == level1[0] || "model" == level1[0])
		{
			level2 = level1[2].split(',');
			glm::vec3 color = glm::vec3(level2[0].toFloat(), level2[1].toFloat(), level2[2].toFloat());

			scene.push_back((GeometryObject*)new Model(level1[1].toStdString(), color));
		}
	}
}