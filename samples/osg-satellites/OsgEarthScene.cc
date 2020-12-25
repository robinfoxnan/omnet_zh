//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#if defined(WITH_OSG) && defined(WITH_OSGEARTH)

#include "OsgEarthScene.h"
#include "ChannelController.h"

#include <osgDB/ReadFile>
#include <osg/PositionAttitudeTransform>
#include <osgEarth/Viewpoint>
#include <osgEarth/MapNode>
#include <osgEarth/Capabilities>
#include <osgEarthAnnotation/RectangleNode>
#include <osgEarth/Version>
#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2, 6, 0)
    #include <osgEarthUtil/Sky>
#else
    #include <osgEarthUtil/SkyNode>
#endif

using namespace osgEarth;
using namespace osgEarth::Annotation;

Define_Module(OsgEarthScene);

OsgEarthScene *OsgEarthScene::instance = nullptr;

OsgEarthScene::OsgEarthScene()
{
    if (instance)
        throw cRuntimeError("There can be only one OsgEarthScene instance in the network");
    instance = this;
}

OsgEarthScene::~OsgEarthScene()
{
    for (int i =0; i<5; i++)
    {
       // delete grid_h[i];
     }
    instance = nullptr;
}

void OsgEarthScene::initialize()
{
    // 下一行读的是：*.osgEarthScene.scene = "simple.earth"
    // 这个文件就是osgearth的配置文件
    auto mapNode = dynamic_cast<osgEarth::MapNode*>(osgDB::readNodeFile(par("scene")));
    if (!mapNode)
        throw cRuntimeError("Could not read scene file \"%s\"", par("scene").stringValue());

    cOsgCanvas *builtinOsgCanvas = getParentModule()->getOsgCanvas();

    builtinOsgCanvas->setCameraManipulatorType(cOsgCanvas::CAM_TRACKBALL);

    // set up viewer
    builtinOsgCanvas->setViewerStyle(cOsgCanvas::STYLE_EARTH);
    builtinOsgCanvas->setClearColor(cOsgCanvas::Color("black"));
    // unit is meters
    builtinOsgCanvas->setZNear(10000);  //6371KM
    builtinOsgCanvas->setZFar(1000000000);

    // 控制地球自转的
    earthRotator = new osg::PositionAttitudeTransform();
    earthRotator->addChild(mapNode);

    scene = new osg::Group();
    scene->asGroup()->addChild(earthRotator);
    #if OSGEARTH_VERSION_GREATER_OR_EQUAL(2, 6, 0)
        scene->asGroup()->addChild(osgEarth::Util::SkyNode::create(mapNode));
    #else
        scene->asGroup()->addChild(new osgEarth::Util::SkyNode(mapNode->getMap()));
    #endif
    auto stateSet = scene->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    mapNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

    builtinOsgCanvas->setScene(scene);

    // one hour in the simulation will be one second in reality (with 1.0 playback speed)
    // 3600 / 360 = 10 , log(10) = 1.0
    getParentModule()->getCanvas()->setAnimationSpeed(360, this);

    // 2维格子数据
    // 拖尾数据，缺少点，需要在运动时候计算
    cCanvas *canvas = getParentModule()->getCanvas();
    canvas->setBackgroundColor(omnetpp::cAbstractImageFigure::WHITE);

    const char * pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 0);
    this->width = atoi(pStr);
    pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 1);
    this->height = atoi(pStr);

    cPolylineFigure *grid_h1 = check_and_cast<cPolylineFigure *>(canvas->getFigure("grid"));

    cFigure::Point left = cFigure::Point(0, height / 2.0);
    cFigure::Point right = cFigure::Point(width, height / 2.0);

    // 维度格子线
    for (int i=0; i< 5; i++)
    {
        grid_h[i] = grid_h1->dup();
        canvas->addFigure(grid_h[i]);


        left = cFigure::Point(0, (i + 1) * height / 6.0);
        right = cFigure::Point(width, (i + 1) *height / 6.0);

        grid_h[i]->addPoint(left );
        grid_h[i]->addPoint(right );
    }

    // 经度格子线
    for (int j=0; j<11; j++)
    {
        grid_v[j] = grid_h1->dup();
        canvas->addFigure(grid_v[j]);

        left =  cFigure::Point((j + 1)* width / 12.0, 0);
        right = cFigure::Point((j + 1)* width / 12.0, height);

        grid_v[j]->addPoint(left );
        grid_v[j]->addPoint(right );
    }



}

OsgEarthScene *OsgEarthScene::getInstance()
{
    if (!instance)
        throw cRuntimeError("OsgEarthScene::getInstance(): there is no OsgEarthScene module in the network");
    return instance;
}

/*
 * osg::PositionAttiruteTransform类没有像osg::MatrixTransform那样的rotate方法，
 * 只有setAttitude方法，该方法的参数是一个四元组对象，四元组的第一个参数是旋转的角度（弧度为单位），
 * 第二个参数是个向量，表示绕哪个轴旋转的向量
 * 23小时，56分， 4秒
 */
void OsgEarthScene::refreshDisplay() const
{
    // 按秒计算当前转过的角度，
    //double angle = simTime().inUnit(SIMTIME_S) / 60.0 / 60 / 24 * 2 * M_PI;
    double angle = simTime().inUnit(SIMTIME_S) * 2.0 * M_PI / ((23.0 * 60 + 56) *60 + 4.0) ;

    earthRotator->setAttitude(osg::Quat(angle, osg::Vec3d(0, 0, 1)));
}
#endif // WITH_OSG
