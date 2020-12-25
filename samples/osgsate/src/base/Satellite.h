//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __MOBILENODE_H__
#define __MOBILENODE_H__

#include <omnetpp.h>
#include "OsgEarthScene.h"

#include <osgEarth/MapNode>
#include <osgEarth/GeoTransform>
#include <osgEarthAnnotation/CircleNode>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthAnnotation/LabelNode>
#include <osgEarthSymbology/style>
#include "Norad.h"
#include "TleFile.h"

using namespace omnetpp;

/**
 * Satellite model. See NED file for details.
 */
class Satellite : public cSimpleModule
{
  protected:
    // configuration
    double startingPhase;
    int startingPhaseDegree;
    std::string labelColor;
    std::string modelURL;
    double modelScale;
    bool bShowCone;

    // 做旋转用的
    osg::ref_ptr<osg::PositionAttitudeTransform> pat;

    // the node containing the osgEarth data
    osg::observer_ptr<osgEarth::MapNode> mapNode = nullptr;
    // osgEarth node for 3D visualization
    osgEarth::GeoTransform *geoTransform = nullptr;

    // 地心引力常数，地球半径
    const double mu = 398600.4418; // "geocentric gravitational constant" - source: wikipedia, units: km^3 / s^2
    const double earthRadius = 6371; // in km


    // 升交点经度
    double ascendingLongitude = 0.0;
    // 轨道面倾角
    double planeAngle = 55.0;

    // 弧度
    double phase = 0; // on the orbit, in radians, unbounded

    std::string name1;
    std::string label;
    osgEarth::Annotation::LabelNode *labelNode;

    // 卫星名字显示的颜色
    osgEarth::Symbology::Style labelStyle;
    osgEarth::Symbology::Style labelStyledown;
    // 状态是否改变
    bool isStatusChanged = true;

    // 轨道编号
    std::string orbitId;

    cPolylineFigure *orbitTail;
    int taillength = 2000;

    // TLE文件中的索引
    int iNum = -1;

    // 每次计算后，获得当前的位置信息，下面几个指标都更新了
    osg::Vec3d pos;
    double longitude, latitude, altitude;

    // 二维平面计算星下点
    std::string tooltip = "";
    int platx, platy;
    int last_x;
    int width, height;

    osg::Vec3d startingPos;
    simtime_t lastPositionUpdateTime = -1;

    // 保存当前参数的控制
    Norad norad;
    simtime_t nextChange;

  public:
    static TleFile tlefile;
    static bool getInfo(int index, std::string & line0, std::string & line1, std::string & line2);
    void ShowInfo();

  public:
    osg::Node *getLocatorNode() { return geoTransform; };
    void updatePosition();
    std::string getOrbitId() { return orbitId; }
    double getStartingPhase() { return startingPhaseDegree; }
    osg::Vec3d getPos() { return  startingPos; }

    // robin added 一些通用的虚函数，比如开关机等，
    bool isWorking = true;
    int handleUserCmd(char * cmd, char* param) override;

    // 计算当前转过的角度，再用地球转过的角度修正
    double getDeltaLongitude(double x);

    osg::Vec3d getPositionAtTime(simtime_t& targetTime);


  protected:
    // angular velocity in rad/sec
    // 圆形的平均角速度为2pi / T ，这里的Gm 是直接计算好的Gm
    double getOmega() { return std::sqrt(mu / std::pow(altitude + earthRadius, 3)); }
    // in world coordinates, units is meters, phase is the angle on the orbit in radians

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return 2; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

};

#endif
