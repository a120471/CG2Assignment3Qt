#include "assignment3qt.h"

#include <thread>
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

	this->show();
	QCoreApplication::processEvents();
	on_pushButton_Render_clicked();
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
	// ALL COLORS ARE stored in RGB CHANNELS

	// create light
	vector<LightBase*> light;
	//light.push_back((LightBase*)new PointLight(glm::vec3(1.3, -1.3, -1.3), glm::vec3(1, 1, 1) * 0.7f));
	//light.push_back((LightBase*)new PointLight(glm::vec3(0.8, -0.5, 3.3), glm::vec3(0.4, 0.6, 0.5) * 1.0f));
	light.push_back((LightBase*)new CubeMap("../cubeMap.hdr", 12.1f));

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
	this->ui.pushButton_Render->repaint();
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

void Assignment3Qt::RenderPixels(RayTracingCameraClass* camera, int row, int start, int end, vector<glm::vec3> &pixelList,
	vector<GeometryObject*> &scene, vector<LightBase*> &light)
{
	vector<RayClass*> rayList;
	RayHitObjectRecord curRayRecord;
	int arrayIdx = row * camera->getW() + start;
	
	for (int col = start; col < end; col++)
	{
		camera->GenerateRay(row, col, rayList);
		// for each ray inside a pixel
		pixelList[arrayIdx] = glm::vec3();
		for (vector<RayClass*>::iterator i = rayList.begin(); i != rayList.end(); i++)
		{
			// find the hit object and hit type
			int hitType = RayHitTest(*i, scene, light, curRayRecord);
			if (hitType == 1)
				pixelList[arrayIdx] += calColorOnHitPoint(curRayRecord, scene, light, 1);
			else if (hitType == 2)
				pixelList[arrayIdx] += curRayRecord.pointColor;

			safe_delete(*i);
		}
		rayList.clear();

		pixelList[arrayIdx] /= camera->getRayNumEachPixel();

		arrayIdx++;
	}
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
	vector<glm::vec3> pixelList;
	pixelListR.resize(pixelNum);
	pixelListG.resize(pixelNum);
	pixelListB.resize(pixelNum);
	pixelList.resize(pixelNum);

	//#pragma omp parallel for
	for (int row = 0; row < camera->getH(); row++)
	{
		// use multi threads, when each task is not heavy, multi thread will even slow down the process
		std::thread processPixel[MYTHREADNUM];

		int start = 0, end = camera->getW() / MYTHREADNUM;
		for (int i = 0; i < MYTHREADNUM; i++)
		{
			processPixel[i] = std::thread(&Assignment3Qt::RenderPixels, this, camera, row, start, end, std::ref(pixelList), scene, light);
			start = end;
			end += camera->getW() / MYTHREADNUM;
			if (i == MYTHREADNUM - 1)
				end = camera->getW() - 1;
		}

		// Join the threads
		for (int i = 0; i < MYTHREADNUM; i++)
			processPixel[i].join();

		int arrayIdx = row * camera->getW();
		for(int col = 0; col < camera->getW(); col++)
		{
			pixelListR[arrayIdx] = pixelList[arrayIdx][0];
			pixelListG[arrayIdx] = pixelList[arrayIdx][1];
			pixelListB[arrayIdx] = pixelList[arrayIdx][2];

			float localScale = 150;
			int R = min((int)(pixelList[arrayIdx][0] * localScale), 255);
			int G = min((int)(pixelList[arrayIdx][1] * localScale), 255);
			int B = min((int)(pixelList[arrayIdx][2] * localScale), 255);
			for (int rowI = 0; rowI < qtIP.imageScaleRatio; rowI++)
				for (int colI = 0; colI < qtIP.imageScaleRatio; colI++)
					qImage->setPixel(col * qtIP.imageScaleRatio + colI, row * qtIP.imageScaleRatio + rowI, qRgb(R, G, B));

			arrayIdx++;
		}

		ui.label_Image->setPixmap(QPixmap::fromImage(*qImage));
		this->ui.label_Image->repaint();
		QCoreApplication::processEvents();
	}

	// calculate the scale ratio
	int nthIdx = pixelNum * NTHIDX;
	nth_element(pixelListR.begin(), pixelListR.begin() + nthIdx, pixelListR.end());
	nth_element(pixelListG.begin(), pixelListG.begin() + nthIdx, pixelListG.end());
	nth_element(pixelListB.begin(), pixelListB.begin() + nthIdx, pixelListB.end());

	float maxRadiance = glm::max(0.01f, pixelListR[nthIdx]);
	maxRadiance = glm::max(maxRadiance, pixelListG[nthIdx]);
	maxRadiance = glm::max(maxRadiance, pixelListB[nthIdx]);
	float scale = 255.0f / maxRadiance;

	int arrayIdx = 0;
	for (int row = 0; row < camera->getH(); row++)
	{
		for (int col = 0; col < camera->getW(); col++)
		{
			pixelList[arrayIdx] *= 100.0f;
			int R = min((int)pixelList[arrayIdx][0], 255);
			int G = min((int)pixelList[arrayIdx][1], 255);
			int B = min((int)pixelList[arrayIdx][2], 255);
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
		if (tmpRecord.depth - lightDis > -MYEPSILON) // the object is further than the light source
			continue;
		if (tmpRecord.depth > MYEPSILON && (record.depth > tmpRecord.depth || record.depth < MYEPSILON))
		{
			record = tmpRecord;
			hitType = 1;
		}
		if (lightDis != MYINFINITE && hitType != 0)
			return hitType;
	}
	if (lightDis == MYINFINITE)
	{
		for (vector<LightBase*>::iterator j = light.begin(); j != light.end(); j++)
		{
			(*j)->RayIntersection(ray, tmpRecord);
			if (tmpRecord.depth > MYEPSILON && (record.depth > tmpRecord.depth || record.depth < MYEPSILON))
			{
				record = tmpRecord;
				hitType = 2;
			}
		}
	}

	return hitType;
}

float diffuseStrength = 0.9f;
float specularStrength = 1.0f - diffuseStrength;
float levelDegenerateRatio = 0.3f;
glm::vec3 Assignment3Qt::calColorOnHitPoint(RayHitObjectRecord &record, vector<GeometryObject*> &scene, vector<LightBase*> &light, int level)
{
	// level starts from 1
	if (level > 3)
		return glm::vec3(0, 0, 0);

	glm::vec3 diffuse(0.0f);
	glm::vec3 specular(0.0f);
	
	glm::vec3 reflectionColor = glm::vec3(0, 0, 0);
	RayClass* reflectionRay = new RayClass(record.hitPoint, record.rDirection);
	RayHitObjectRecord reflectionHitRecord;
	int hitType = RayHitTest(reflectionRay, scene, light, reflectionHitRecord);
	if (hitType == 1)
	{
		glm::vec3 recursiveHitPointColor = calColorOnHitPoint(reflectionHitRecord, scene, light, level + 1);
		reflectionColor = levelDegenerateRatio * max(dot(record.hitNormal, record.rDirection), 0.0f) * recursiveHitPointColor;
	}
	else if (hitType == 2)
	{
		specular += specularStrength * reflectionHitRecord.pointColor;
	}
	safe_delete(reflectionRay);

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

		for (unsigned int j = 0; j < lightDirList.size(); j++)
		{
			RayClass* lightRay = new RayClass(record.hitPoint, lightDirList[j]);
			if (!RayHitTest(lightRay, scene, light, lightHitRecord, lightDisList[j]))
			{
				float diff = max(dot(record.hitNormal, lightDirList[j]), 0.0f);
				diffuse += diffuseStrength * diff * lightColorList[j];
			}

			safe_delete(lightRay);
		}
	}

	glm::vec3 returnColor = glm::vec3(0);
	returnColor += diffuse / (float)lightDirList.size() * 1.0f + specular;
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