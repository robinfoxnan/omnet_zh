//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __OSGRENDERER_H__
#define __OSGRENDERER_H__

#include <omnetpp.h>

#include <osg/Node>
#include <osgEarth/MapNode>
#include <osgText/Text>

using namespace omnetpp;

/**
 * Initialize global 3D canvas and load the configured Earth model file.
 */
class OsgEarthScene : public cSimpleModule
{
  protected:
    static OsgEarthScene *instance;
    osg::ref_ptr<osg::Node> scene;

    // 地球自转控制器
    osg::PositionAttitudeTransform *earthRotator = nullptr;

    cPolylineFigure *grid_h[5];

    cPolylineFigure *grid_v[11];

    cTextFigure *textFigure;
    time_t tmTarget;
    osg::ref_ptr<osgText::Text> pText = 0;

    int width = 1080;
    int height = 1080;

    // 默认OSG的0经度和地球0经度重合，需要根据目标时间，确定地球的默认角度；
    // 这里使用弧度
    double gap = 0;

  public:
    OsgEarthScene();
    virtual ~OsgEarthScene();

    void setGap(double deltaDegree);
    void addHudtime();

    static OsgEarthScene *getInstance();
    virtual osg::Node *getScene() { return scene; }
  protected:
    virtual void initialize() override;
    virtual void refreshDisplay() const override;
};

#endif
