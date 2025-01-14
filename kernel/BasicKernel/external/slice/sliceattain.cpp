﻿#include "sliceattain.h"
#include "cxgcode/gcodehelper.h"
#include "gcode/gcodedata.h"
#include "data/kernelmacro.h"

#include "interface/machineinterface.h"
#include "interface/appsettinginterface.h"

#include "qtusercore/string/resourcesfinder.h"
#include "qtusercore/module/systemutil.h"

#include <Qt3DRender/QAttribute>
#include <QtCore/QUuid>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QThread>

using namespace qtuser_3d;
namespace creative_kernel
{
	SliceAttain::SliceAttain(SliceResultPointer result, SliceAttainType type, QObject* parent)
		:QObject(parent)
		, m_cache(nullptr)
		, m_attribute(nullptr)
		, m_type(type)
		, m_result(result)
	{
		QString path = QString("%1/%2").arg(TEMPGCODE_PATH).arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
		mkMutiDir(path);

		QString name = QString(QString::fromLocal8Bit(m_result->sliceName().c_str()));
		m_tempGCodeFileName = QString("%1/%2").arg(path).arg(name);
		int maxPath = qtuser_core::getSystemMaxPath() - 7;
		if (m_tempGCodeFileName.length() > maxPath)
		{
			m_tempGCodeFileName = m_tempGCodeFileName.left(maxPath);
		}
		m_tempGCodeImageFileName = QString("%1_image").arg(m_tempGCodeFileName);
		if (m_tempGCodeImageFileName.length() > maxPath)
		{
			m_tempGCodeImageFileName = QString("%1_image").arg(m_tempGCodeImageFileName.left(maxPath - 6));
		}
		m_tempGCodeFileName += ".gcode";
		m_tempGCodeImageFileName += ".gcode";
	}

	SliceAttain::~SliceAttain()
	{
		QFileInfo info(m_tempGCodeFileName);
		QString path = info.absolutePath();

		clearPath(path);
		QDir dir;
		dir.setPath(path);
		dir.removeRecursively();
	}

	void SliceAttain::build(ccglobal::Tracer* tracer)
	{
		builder.build(m_result, tracer);
	}

	QString SliceAttain::sliceName()
	{
		
		return QString(QString::fromLocal8Bit(m_result->sliceName().c_str()));
	}

	QString SliceAttain::sourceFileName()
	{
		return QString(QString::fromLocal8Bit(m_result->fileName().c_str()));
	}

	bool SliceAttain::isFromFile()
	{
		return m_type == SliceAttainType::sat_file;
	}

	int SliceAttain::printTime()
	{
		return builder.parseInfo.printTime;
	}

	QString SliceAttain::material_weight()
	{
		return QString::number(builder.parseInfo.weight);
	}

	QString SliceAttain::printing_time()
	{
		int printTime = builder.parseInfo.printTime;
		QString str = QString("%1h%2m%3s").arg(printTime / 3600)
			.arg(printTime / 60 % 60)
			.arg(printTime % 60);
		return str;
	}

	QString SliceAttain::material_money()
	{
		return QString::number(builder.parseInfo.cost);
	}

	QString SliceAttain::material_length()
	{
		return QString::number(builder.parseInfo.materialLenth);
	}

	trimesh::box3 SliceAttain::box()
	{
		return m_result->inputBox;
	}

	int SliceAttain::layers()
	{
		return builder.baseInfo.layers;
	}

	int SliceAttain::steps(int layer)
	{
		int _layer = layer - INDEX_START_AT_ONE;

		if (_layer < 0 || _layer >= (int)builder.baseInfo.steps.size())
			return 0;

		return builder.baseInfo.steps.at(_layer);
	}

	int SliceAttain::totalSteps()
	{
		return builder.baseInfo.totalSteps;
	}

	int SliceAttain::nozzle()
	{
		return builder.baseInfo.nNozzle;
	}

	float SliceAttain::minSpeed()
	{
		return builder.baseInfo.speedMin;
	}

	float SliceAttain::maxSpeed()
	{
		return builder.baseInfo.speedMax;
	}

	float SliceAttain::minTimeOfLayer()
	{
		return builder.baseInfo.minTimeOfLayer;
	}

	float SliceAttain::maxTimeOfLayer()
	{
		return builder.baseInfo.maxTimeOfLayer;
	}

	float SliceAttain::minFlowOfStep()
	{
		return builder.baseInfo.minFlowOfStep;
	}

	float SliceAttain::maxFlowOfStep()
	{
		return builder.baseInfo.maxFlowOfStep;
	}

	float SliceAttain::minLineWidth()
	{
		return builder.baseInfo.minLineWidth;
	}

	float SliceAttain::maxLineWidth()
	{
		return builder.baseInfo.maxLineWidth;
	}

	float SliceAttain::minLayerHeight()
	{
		return builder.baseInfo.minLayerHeight;
	}

	float SliceAttain::maxLayerHeight()
	{
		return builder.baseInfo.maxLayerHeight;
	}

	float SliceAttain::minTemperature()
	{
		return builder.baseInfo.minTemperature;
	}

	float SliceAttain::maxTemperature()
	{
		return builder.baseInfo.maxTemperature;
	}

	float SliceAttain::layerHeight()
	{
		return builder.parseInfo.layerHeight;
	}

	float SliceAttain::lineWidth()
	{
		return builder.parseInfo.lineWidth;
	}

	gcode::TimeParts SliceAttain::getTimeParts() const {
		return builder.parseInfo.timeParts;
	}

	QImage* SliceAttain::getImageFromGcode()
	{
		if (m_result)
		{
			if (!m_result->previewsData.empty())
			{
				QImage* image=new QImage();
				image->loadFromData(&m_result->previewsData.back()[0], m_result->previewsData.back().size());
				return image;
			}
		}
		return nullptr;
	}

	QString SliceAttain::fileNameFromGcode()
	{
		if (m_result)
		{
			QString str = QString(QString::fromLocal8Bit(m_result->sliceName().c_str()));
			int index = str.lastIndexOf('/');
			if (index)
			{
				str = str.right(str.length() - index-1);
			}
			int index2 = str.lastIndexOf(".gcode");
			if (index2)
			{
				str = str.left(index2);
			}
			return str;
		}

		return "";
	}

	float SliceAttain::traitSpeed(int layer, int step)
	{
		return builder.traitSpeed(layer, step);
	}

	trimesh::vec3 SliceAttain::traitPosition(int layer, int step)
	{
		return builder.traitPosition(layer, step);
	}

	trimesh::fxform SliceAttain::modelMatrix()
	{
		return builder.parseInfo.xf4;
	}

	int SliceAttain::findViewIndexFromStep(int layer, int nStep)
	{
		int _layer = layer - INDEX_START_AT_ONE;
		int _nStep = nStep - INDEX_START_AT_ONE;

		if (_layer >= 0 && _layer < layers())
		{
			const std::vector<int>& maps = builder.m_stepGCodesMaps.at(_layer);
			if (_nStep >= 0 && _nStep < maps.size())
				return maps.at(_nStep);
		}
		return -1;
	}

	int SliceAttain::findStepFromViewIndex(int layer, int nViewIndex)
	{
		int _layer = layer - INDEX_START_AT_ONE;
		if (_layer >= 0 && _layer < layers())
		{
			const std::vector<int>& maps = builder.m_stepGCodesMaps.at(_layer);
			if (nViewIndex >= maps.size()) return -1;
			return maps.at(nViewIndex);
		}
		return -1;
	}

	void SliceAttain::updateStepIndexMap(int layer)
	{
	}

	const QString SliceAttain::layerGcode(int layer)
	{
		int _layer = layer - INDEX_START_AT_ONE;
		return QString::fromStdString(m_result->layer(_layer));
	}

	void SliceAttain::setGCodeVisualType(gcode::GCodeVisualType type)
	{
		builder.updateFlagAttribute(m_attribute, type);
	}

	Qt3DRender::QGeometry* SliceAttain::createGeometry()
	{
		if (!m_cache)
		{
			m_cache = builder.buildGeometry();
			m_attribute = new Qt3DRender::QAttribute(m_cache);
			m_cache->addAttribute(m_attribute);
		}

		return m_cache;
	}

    float SliceAttain::getMachineHeight()
    {
        return builder.parseInfo.machine_height;
    }

    float SliceAttain::getMachineWidth()
    {
        return builder.parseInfo.machine_width;
    }

    float SliceAttain::getMachineDepth()
    {
        return builder.parseInfo.machine_depth;
    }

    int SliceAttain::getBeltType()
    {
        return builder.parseInfo.beltType;
    }

	Qt3DRender::QGeometry* SliceAttain::createRetractionGeometry()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildRetractionGeometry();
#else
		return nullptr;
#endif
	}

	Qt3DRender::QGeometry* SliceAttain::createZSeamsGeometry()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildZSeamsGeometry();
#else
		return nullptr;
#endif
	}

	Qt3DRender::QGeometryRenderer* SliceAttain::createRetractionGeometryRenderer()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildRetractionGeometryRenderer();
#else
		return nullptr;
#endif
	}

	Qt3DRender::QGeometryRenderer* SliceAttain::createZSeamsGeometryRenderer()
	{
#if SHOW_ZSEAM_POINTS
		return builder.buildZSeamsGeometryRenderer();
#else
		return nullptr;
#endif
	}

	void SliceAttain::saveGCode(const QString& fileName, QImage* previewImage)
	{
		QString imageStr;
		if (previewImage && !isFromFile())
		{
			float layerHeight = builder.parseInfo.layerHeight;
			QString screenSize = QString(QString::fromLocal8Bit(builder.parseInfo.screenSize.c_str()));
            QString exportFormat = QString(QString::fromLocal8Bit(builder.parseInfo.exportFormat.c_str()));

			previewImage = cxsw::resizeModule(previewImage);
			if (exportFormat == "bmp")
			{
				cxsw::image2String(*previewImage, 64, 64, true, imageStr);
				cxsw::image2String(*previewImage, 400, 400, false, imageStr);
			}
			else
			{
				QImage minPreImg;
				QImage maxPreImg;
				if (screenSize == "CR-10 Inspire")
				{
					minPreImg = previewImage->scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					maxPreImg = previewImage->scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				else if (screenSize == "CR-200B Pro")
				{
					minPreImg = previewImage->scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					maxPreImg = previewImage->scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				else if (screenSize == "Ender-3 S1")
				{
					maxPreImg = previewImage->scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				else if (screenSize == "Ender-5 S1")
				{
					maxPreImg = previewImage->scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				else//Sermoon D3
				{
					minPreImg = previewImage->scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					maxPreImg = previewImage->scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				}
				if (!minPreImg.isNull())
				{
					cxsw::getImageStr(imageStr, &minPreImg, builder.baseInfo.layers, exportFormat, layerHeight, false, SLICE_PATH);
				}
				cxsw::getImageStr(imageStr, &maxPreImg, builder.baseInfo.layers, exportFormat, layerHeight, exportFormat == "png", SLICE_PATH);
			}
		}
		else if (isFromFile())
		{
			float layerHeight = builder.parseInfo.layerHeight;
			QString screenSize = QString(QString::fromLocal8Bit(builder.parseInfo.screenSize.c_str()));
			QString exportFormat = QString(QString::fromLocal8Bit(builder.parseInfo.exportFormat.c_str()));

			QString imgSavePath = QString("%1/imgPreview.%2").arg(SLICE_PATH).arg(exportFormat);
			QImage* image = getImageFromGcode();
			if(image)
				image->save(imgSavePath);
			else
			{
				QFile::remove(imgSavePath);
			}
			//cxsw::getImageStr(imageStr, getImageFromGcode(), builder.baseInfo.layers, exportFormat, layerHeight, false, SLICE_PATH);
		}

		cxsw::_SaveGCode(fileName.toLocal8Bit().data(), imageStr.toLocal8Bit().data(), m_result->layerCode(), m_result->prefixCode(), m_result->tailCode());
	}

	void SliceAttain::saveTempGCode()
	{
		saveGCode(m_tempGCodeFileName, nullptr);
	}

	void SliceAttain::saveTempGCodeWithImage(QImage& image)
	{
		saveGCode(m_tempGCodeImageFileName, &image);
	}

	QString SliceAttain::tempGCodeFileName()
	{
        return m_tempGCodeFileName;
    }

    QString SliceAttain::tempGcodeThumbnail()
    {
		QString exportFormat = QString(QString::fromLocal8Bit(builder.parseInfo.exportFormat.c_str()));
        return QString("%1/imgPreview.%2").arg(SLICE_PATH).arg(exportFormat);
    }

	QString SliceAttain::tempGCodeImageFileName()
	{
		if (m_tempGCodeImageFileName.isEmpty())
			return m_tempGCodeFileName;
		return m_tempGCodeImageFileName;
	}

	QString SliceAttain::tempImageFileName()
	{
		return QString("%1/slice_flow_gcode_preview.png").arg(SLICE_PATH);
	}

	void SliceAttain::getPathData(const trimesh::vec3 point, float e, int type)
	{
		builder.getPathData(point,e, type);
	}
	void SliceAttain::getPathDataG2G3(const trimesh::vec3 point, float i, float j, float e, int type, bool isG2)
	{
		builder.getPathDataG2G3(point,i,j,e,type,isG2);
	}
	void SliceAttain::setParam(crslice::PathParam pathParam)
	{
		gcode::GCodeParseInfo gcodeParaseInfo;
		gcodeParaseInfo.machine_height = pathParam.machine_height;
		gcodeParaseInfo.machine_width = pathParam.machine_width;
		gcodeParaseInfo.machine_depth = pathParam.machine_depth;
		gcodeParaseInfo.printTime = pathParam.printTime;
		gcodeParaseInfo.materialLenth = pathParam.materialLenth;
		gcodeParaseInfo.material_diameter = pathParam.material_diameter = { 1.75 }; //材料直径
		gcodeParaseInfo.material_density = pathParam.material_density = { 1.24 };  //材料密度
		gcodeParaseInfo.lineWidth = pathParam.lineWidth;
		gcodeParaseInfo.layerHeight = pathParam.layerHeight;
		gcodeParaseInfo.spiralMode = pathParam.spiralMode;
		gcodeParaseInfo.exportFormat = pathParam.exportFormat;//QString exportFormat;
		gcodeParaseInfo.screenSize = pathParam.screenSize;//QString screenSize;

		gcodeParaseInfo.timeParts = {
		pathParam.timeParts.OuterWall,
		pathParam.timeParts.InnerWall,
		pathParam.timeParts.Skin,
		pathParam.timeParts.Support,
		pathParam.timeParts.SkirtBrim,
		pathParam.timeParts.Infill,
		pathParam.timeParts.SupportInfill,
		pathParam.timeParts.MoveCombing,
		pathParam.timeParts.MoveRetraction,
		pathParam.timeParts.PrimeTower };

		gcodeParaseInfo.beltType = pathParam.beltType;  // 1 creality print belt  2 creality slicer belt
		gcodeParaseInfo.beltOffset = pathParam.beltOffset;
		gcodeParaseInfo.beltOffsetY = pathParam.beltOffsetY;
		gcodeParaseInfo.xf4 = pathParam.xf4;//cr30 fxform

		gcodeParaseInfo.relativeExtrude = pathParam.relativeExtrude;
		builder.setParam(gcodeParaseInfo);
	}
	void SliceAttain::setLayer(int layer)
	{
		builder.setLayer(layer);

		qDebug() << "SliceAttain setLayer thread " << QThread::currentThread() << "layer = " << layer;

		if (layer > 0 && layer % 5 == 0)
		{
			//build_preview();
			//emit layerChanged(layer);
		}
	}

	void SliceAttain::setLayers(int layer)
	{
		if (layer >= 0)
			builder.baseInfo.layers = layer;
	}

	void SliceAttain::setSpeed(float s)
	{
		builder.setSpeed(s);
	}
	void SliceAttain::setTEMP(float temp)
	{
		builder.setTEMP(temp);
	}
	void SliceAttain::setExtruder(int nr)
	{
		builder.setExtruder(nr);
	}
	void SliceAttain::setFan(float fan)
	{
		builder.setFan(fan);
	}
	void SliceAttain::setZ(float z, float h)
	{
		builder.setZ(z,h);
	}
	void SliceAttain::setE(float e)
	{
		builder.setE(e);
	}

	void SliceAttain::setTime(float time)
	{
		builder.setTime(time);
	}

	void SliceAttain::getNotPath()
	{
		builder.getNotPath();
	}
}
