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
#include <osgEarthUtil/AutoClipPlaneHandler>
#if OSGEARTH_VERSION_GREATER_OR_EQUAL(2, 6, 0)
    #include <osgEarthUtil/Sky>
#else
    #include <osgEarthUtil/SkyNode>
#endif

//#include <QString>
#include "util.h"

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

// 经度，转换为弧度
void OsgEarthScene::setGap(double deltaDegree)
{
    if (deltaDegree > 0)
    {
        deltaDegree = fmod(deltaDegree, 360);
    }
    else
    {
        deltaDegree += 360;
    }

    this->gap =  deltaDegree * M_PI / 180;
}
void OsgEarthScene::initialize()
{
    // 下一行读的是：*.osgEarthScene.scene = "simple.earth"
    // 这个文件就是osgearth的配置文件
    auto mapNode = dynamic_cast<osgEarth::MapNode*>(osgDB::readNodeFile(par("scene")));
    if (!mapNode)
        throw cRuntimeError("Could not read scene file \"%s\"", par("scene").stringValue());


   // pCamera->addCullCallback( new osgEarth::Util::AutoClipPlaneCullCallback(mapNode)  );

    cOsgCanvas *builtinOsgCanvas = getParentModule()->getOsgCanvas();


    builtinOsgCanvas->setCameraManipulatorType(cOsgCanvas::CAM_TRACKBALL);
   // builtinOsgCanvas->getViewerStyle()

    //viewer.getCamera()->addCullCallback( new osgEarth::Util::AutoClipPlaneCullCallback(mapNode) );

    // set up viewer
    builtinOsgCanvas->setViewerStyle(cOsgCanvas::STYLE_EARTH);
    builtinOsgCanvas->setClearColor(cOsgCanvas::Color("black"));
    // unit is meters
    builtinOsgCanvas->setZNear(10000);  //6378KM
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

    // 仿真时间
    textFigure = check_and_cast<cTextFigure *>(canvas->getFigureByPath("status.TimeInfo"));

    std::string strTime = par("targetTime").stringValue();
    textFigure->setText(strTime.c_str());
    tmTarget = StringToDatetime(strTime);

    // 三维显示时间
    addHudtime();

}

void OsgEarthScene::addHudtime()
{

    // 增加新的文字，增加相机实现hud
    osg::ref_ptr<osg::Geode> pGeode = new osg::Geode();

        // 步骤一：实例化文字类
        pText = new osgText::Text();

            pText = new osgText::Text();
            // "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径
            pText->setFont(osgText::readFontFile("fonts/simhei.ttf"));
            //QString text = QString("2020-10-01 20:01:01");
            std::string text = "仿真时间 2020-10-01 20:01:01";
            pText->setText(text.c_str(), osgText::String::ENCODING_UTF8);
            pText->setPosition(osg::Vec3f(500, 760, 0));
            pText->setCharacterSize(20);
            pText->setColor(osg::Vec4f(1.0, 1.0, 1.0, 1.0));
            pText->setDrawMode(osgText::Text::TEXT);
            // 此条会导致hub不显示（必须设置为SCREEN）
    //        pText->setAxisAlignment(osgText::Text::XZ_PLANE);
            pText->setAxisAlignment(osgText::Text::XY_PLANE);
            pText->setCharacterSizeMode(osgText::Text::OBJECT_COORDS);
            pText->setFontResolution(32, 32);
            pText->setAlignment(osgText::Text::LEFT_BOTTOM);
            pText->setLayout(osgText::Text::LEFT_TO_RIGHT);
            pText->setBackdropType(osgText::Text::NONE);
            pText->setColorGradientMode(osgText::Text::SOLID);

            // 步骤一：创建HUD摄像机
            osg::ref_ptr<osg::Camera> pCamera = new osg::Camera;
            // 步骤二：设置投影矩阵
            pCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, 1280, 0, 800));
            // 步骤三：设置视图矩阵,同时确保不被场景中其他图形位置变换影响, 使用绝对帧引用
            pCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
            pCamera->setViewMatrix(osg::Matrix::identity());
            // 步骤四：清除深度缓存
            pCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
            // 步骤五：设置POST渲染顺序(最后渲染)
            pCamera->setRenderOrder(osg::Camera::POST_RENDER);
            // 步骤六：设置为不接收事件,始终得不到焦点
            pCamera->setAllowEventFocus(false);


 //           pGeode = new osg::Geode();
            osg::ref_ptr<osg::StateSet> pStateSet = pGeode->getOrCreateStateSet();
            // 步骤七：关闭光照
            pStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            // 步骤八：关闭深度测试
            pStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

            pGeode->addDrawable(pText.get());
            pCamera->addChild(pGeode.get());
            scene->asGroup()->addChild(pCamera.get());

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
 * time_t 这种类型就是用来存储从1970年到现在经过了多少秒，要想更精确一点，可以用结构struct timeval，它精确到微妙。
    struct timeval
    {
    long tv_sec;
    long tv_usec;
    };
 */
void OsgEarthScene::refreshDisplay() const
{
    // 按秒计算当前转过的角度，
    //double angle = simTime().inUnit(SIMTIME_S) / 60.0 / 60 / 24 * 2 * M_PI;
    double angle = gap + simTime().inUnit(SIMTIME_S) * 2.0 * M_PI / ((23.0 * 60 + 56) *60 + 4.0) ;

    earthRotator->setAttitude(osg::Quat(angle, osg::Vec3d(0, 0, 1)));

    // 设置时间
    // 设置状态信息
    time_t current = tmTarget + simTime().inUnit(SIMTIME_S);
    string strTime = DatetimeToString(current);
    textFigure->setText(strTime.c_str());

    // 三维时间
    std::string temp = "仿真时间  " + strTime;
    pText->setText(temp, osgText::String::ENCODING_UTF8);
}




#endif // WITH_OSG
