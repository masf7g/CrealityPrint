#ifndef _GCODE_NULLSPACE_SLICEMODELBUILDER_1590033412412_H
#define _GCODE_NULLSPACE_SLICEMODELBUILDER_1590033412412_H
#include "gcode/interface.h"
#include <float.h>
#include <vector>
#include <map>
#include "gcode/sliceline.h"
#include "gcode/sliceresult.h"
#include "gcode/define.h"
#include "ccglobal/tracer.h"
#include "trimesh2/Box.h"

namespace gcode
{
	struct G2G3Info
	{
		float f;
		float e;
		float x;
		float y;
		float i;
		float j;
		float currentE;
		bool isG2;
		bool bIsTravel;
	};

	struct GCodeParseInfo;
	struct GCodeStructBaseInfo
	{
		std::vector<int> layerNumbers;
		std::vector<int> steps;
		int totalSteps;
		int layers;
		float minTimeOfLayer;   //�������ʱ��
		float maxTimeOfLayer;   //�����ʱ��

		float minFlowOfStep;  //������С����������
		float maxFlowOfStep;  //�����������

		float minLineWidth;
		float maxLineWidth;

		float minLayerHeight;
		float maxLayerHeight;

		float minTemperature;
		float maxTemperature;

		trimesh::box3 gCodeBox;
		int nNozzle;
		float speedMin;
		float speedMax;

		GCodeStructBaseInfo()
			: speedMin(FLT_MAX)
			, speedMax(FLT_MIN)
			, layers(0)
			, nNozzle(0)
			, totalSteps(0)
			, minTimeOfLayer(FLT_MAX)
			, maxTimeOfLayer(FLT_MIN)
			, minFlowOfStep(FLT_MAX)
			, maxFlowOfStep(FLT_MIN)
			, minLineWidth(FLT_MAX)
			, maxLineWidth(FLT_MIN)
			, minLayerHeight(FLT_MAX)
			, maxLayerHeight(FLT_MIN)
			, minTemperature(FLT_MAX)
			, maxTemperature(FLT_MIN)
		
		{
		}
	};

	struct GcodeTemperature
	{
		float bedTemperature{ 0.0f }; //ƽ̨�¶�
		float temperature{ 0.0f };  //�����¶�
		float camberTemperature{ 0.0f };  //ǻ���¶�
	};

	struct GcodeFan
	{
		float fanSpeed{ 0.0f }; //�����ٶ�
		float camberSpeed{ 0.0f };  //ǻ������ٶ�
		float fanSpeed_1{ 0.0f };  //�ӷ���
	};

	struct GcodeLayerInfo
	{
		float layerHight{ 0.0f }; //���
		float width{ 0.0f };  //�߿�
		float flow{ 0.0f }; //�������
	};

	struct GCodeMove
	{
		int start;
		float speed;
		SliceLineType type;
		float e;  //����
		int extruder;
	};

	class GCODE_API GCodeStruct
	{
	public:
		GCodeStruct();
		~GCodeStruct();

		void buildFromResult(SliceResultPointer result, const GCodeParseInfo& info, GCodeStructBaseInfo& baseInfo, std::vector<std::vector<int>>& stepIndexMaps, ccglobal::Tracer* tracer = nullptr);
		void buildFromResult(GCodeParseInfo& info, GCodeStructBaseInfo& baseInfo, std::vector<std::vector<int>>& stepIndexMaps, ccglobal::Tracer* tracer = nullptr);
		void buildFromResult(gcode::SliceResult* result, ccglobal::Tracer* tracer = nullptr);

		std::vector<trimesh::vec3> m_positions;
		std::vector<GCodeMove> m_moves;  //�������ٶ�..

		std::vector <GcodeTemperature> m_temperatures;//�¶�����ֵ
		std::vector<int> m_temperatureIndex;//�¶Ȳ�������
		std::vector <GcodeFan> m_fans;//��������ֵ
		std::vector<int> m_fanIndex;//���Ȳ�������
		std::vector <GcodeLayerInfo> m_gcodeLayerInfos;  //��ߡ��߿�����ֵ
		std::vector<int> m_layerInfoIndex;  //��ߡ��߿� ��������

		std::map<int,float> m_layerTimes;  //ÿ��ʱ��
		//std::map<int, float> m_layerTimeLogs;  //ÿ��ʱ�����

		std::vector<int> m_zSeams;
		std::vector<int> m_retractions;

		void getPathData(const trimesh::vec3 point, float e, int type);
		void getPathDataG2G3(const trimesh::vec3 point, float i, float j, float e, int type, bool isG2 = true);
		void setParam(gcode::GCodeParseInfo& pathParam);
		void setLayer(int layer);
		void setSpeed(float s);
		void setTEMP(float temp);
		void setExtruder(int nr);
		void setFan(float fan);
		void setZ(float z, float h);
		void setE(float e);
		void setTime(float time);
		void getNotPath();
	private:
		void processLayer(const std::string& layerCode, int layer, std::vector<int>& stepIndexMap);
		void processStep(const std::string& stepCode, int nIndex, std::vector<int>& stepIndexMap);
		void processG01(const std::string& G01Str, int nIndex, std::vector<int>& stepIndexMap,bool isG2G3 =false);
		void processG01_sub(SliceLineType tempType, double tempEndE, trimesh::vec3 tempEndPos, bool havaXYZ, int nIndex, std::vector<int>& stepIndexMap, bool isG2G3);
		void processG23(const std::string& G23Str, int nIndex, std::vector<int>& stepIndexMap);
		void processG23_sub(G2G3Info info, int nIndex, std::vector<int>& stepIndexMap);
		void processSpeed(float speed);

		void processPrefixCode(const std::string& stepCod);
		void checkoutFan(const std::string& stepCod);
		void checkoutTemperature(const std::string& stepCode);
		void checkoutLayerInfo(const std::string& stepCode,int layer);
		void checkoutLayerHeight(const std::vector<std::string>& layerLines);
	protected:
		SliceLineType  tempCurrentType;
		int tempNozzleIndex;
		double tempCurrentE;
		float tempCurrentTime{ 0.0f };
		float tempCurrentZ{ 0.0f };
		float belowZ{ 0.0f };//�ϲ���
		trimesh::vec3 tempCurrentPos;
		float tempSpeed;
		float tempSpeedMax{ 0.0f };//����ٶ�����
		int tempTempIndex{ 0 };; //��ǰ�¶�����
		bool layerNumberParseSuccess;

		int nIndex = 0;

		int startNumber = 0;
		std::vector<std::vector<int>> m_stepIndexMaps;

		GCodeParseInfo parseInfo;
		GCodeStructBaseInfo tempBaseInfo;

		ccglobal::Tracer* m_tracer;
	};
}
#endif // _GCODE_NULLSPACE_SLICEMODELBUILDER_1590033412412_H