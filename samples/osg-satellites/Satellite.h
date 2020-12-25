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

    double altitude = 10000; // in km, above the surface

    // 升交点经度
    double ascendingLongitude = 0.0;
    // 轨道面倾角
    double planeAngle = 55.0;

    // 弧度
    double phase = 0; // on the orbit, in radians, unbounded

    std::string name;
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

    // 轨道的法线向量
    osg::Vec3d normal = osg::Vec3d(0, 0, 1); // doesn't have to be unit length, just can't be 0

    // 这里是将轨道平面的X,Y轴使用2个向量重新定义了，这样法线就相当于新的Z轴，这样计算起来方便
    osg::Vec3d orbitX, orbitY; // the base of the orbit plane, orthogonal, and both are unit length, computed from the normal
    osg::Vec3d pos;

    // 二维平面计算星下点
    int x, y;
    int last_x;
    int x0, y0;
    int width, height;

    osg::Vec3d startingPos;
    simtime_t lastPositionUpdateTime = -1;

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


  protected:
    // angular velocity in rad/sec
    // 圆形的平均角速度为2pi / T ，这里的Gm 是直接计算好的Gm
    double getOmega() { return std::sqrt(mu / std::pow(altitude + earthRadius, 3)); }
    // in world coordinates, units is meters, phase is the angle on the orbit in radians
    osg::Vec3 getPositionAtPhase(double alpha) const;
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return 2; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

};

#endif
