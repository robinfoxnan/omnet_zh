//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#if defined(WITH_OSG) && defined(WITH_OSGEARTH)

#include "Satellite.h"
#include "OsgEarthScene.h"
#include "ChannelController.h"

#include <sstream>
#include <iomanip>

#include "omnetpp/osgutil.h"

#include <osg/Node>
#include <osg/Texture>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/Depth>
#include <osg/PositionAttitudeTransform>
#include <osgEarth/Capabilities>
#include <osgEarthAnnotation/LabelNode>
#include <osgEarthSymbology/Geometry>
#include <osgEarthFeatures/Feature>
// new added
#include <osg/CoordinateSystemNode>
#include <osg/PositionAttitudeTransform>
#include "util.h"

using namespace omnetpp;

Define_Module(Satellite)

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;


// 初始化静态变量
TleFile Satellite::tlefile = TleFile();

bool Satellite::getInfo(int index, std::string & line0, std::string & line1, std::string & line2)
{
    return tlefile.getInfo(index, line0, line1, line2);

}

// robin added 一些通用的虚函数，比如开关机等，
int Satellite::handleUserCmd(char * cmd, char* param)
{
    isStatusChanged = true;
        if (strcmp(cmd, "STOP") == 0)
        {
            isWorking = false;
            return 1;
        }
        else if (strcmp(cmd, "START") == 0)
        {
            isWorking = true;
            return 2;
        }
        return -1;
}
//////////////////////////////
// 经纬度转XYZ坐标
osg::Vec3d LonLatAltToWorld(const osg::Vec3d lonLatAlt)
{
    osg::Vec3d world;
    osg::EllipsoidModel* em = new osg::EllipsoidModel();
    //Lat纬度Long经度
    em->convertLatLongHeightToXYZ(osg::DegreesToRadians(lonLatAlt.y()),
    osg::DegreesToRadians(lonLatAlt.x()), lonLatAlt.z(), world.x(), world.y(), world.z());

    return world;
}

// 世界坐标转经纬度
osg::Vec3d WorldToLonLatAlt(const osg::Vec3d world)
{
    osg::EllipsoidModel* em = new osg::EllipsoidModel();
    osg::Vec3d lonLatAlt;
    em->convertXYZToLatLongHeight(world.x(), world.y(), world.z(),
    lonLatAlt.y(), lonLatAlt.x(), lonLatAlt.z());

    lonLatAlt.x() = osg::RadiansToDegrees(lonLatAlt.x());
    lonLatAlt.y() = osg::RadiansToDegrees(lonLatAlt.y());
    return lonLatAlt;
}

// 输入三个世界坐标点，计算法线
osg::Vec3d ComputeNormal(osg::Vec3d point1, osg::Vec3d point2, osg::Vec3d point3)
{
    // 第1根线
    osg::Vec3d vVector1 = point1 - point2;

    // 第2根线
    osg::Vec3d vVector2 = point1- point3;

    osg::Vec3d vNormal = vVector1 ^  vVector2;

    // 规范化
    vNormal.normalize();
    return vNormal;

}

// GEO 和IGSO，倾斜地球同步轨道(Inclined GeoSynchronous Orbit) 计算法线向量
// 地心是(0,  0 , 0)
// 升交点(ascendingLongitude, 0, altitude * 1000 )      Longitude of ascending node
// 高点：在升交点和将交点连线相垂直，过地心的垂面与轨道交点；
//       经度 = 升交点经度 + pi / 2;
//       海拔 = altitude * 1000
//       纬度 = 倾角i
// 3个点就可以计算法线向量了，
osg::Vec3d computeNormalForIGSO(double altitude, double i, double ascendingLongitude)
{
    // 地心
    osg::Vec3d point0 = osg::Vec3d(0, 0, 0);

    // 升交点
    osg::Vec3d point1 = osg::Vec3d(ascendingLongitude, 0, altitude * 1000);
    osg::Vec3d point1XYZ =  LonLatAltToWorld(point1);

    // 高点
    osg::Vec3d point2 = osg::Vec3d(ascendingLongitude + 90, i, altitude * 1000);
    osg::Vec3d point2XYZ =  LonLatAltToWorld(point2);

    return  ComputeNormal(point0, point1XYZ, point2XYZ);
}


// 绘制XYZ轴，为了方便调试和理解；
osg::ref_ptr<osg::Node> makeCoordinate() {
    //创建保存几何信息的对象
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

    //创建四个顶点
    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
    double r = (6371 + 35786 + 3000) * 1000;
    v->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    v->push_back(osg::Vec3(r, 0.0f, 0.0f));

    v->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    v->push_back(osg::Vec3(0.0f, r, 0.0f));

    v->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    v->push_back(osg::Vec3(0.0f, 0.0f, r));

    // 加一个0经度，看看在哪里
    v->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    v->push_back(LonLatAltToWorld(osg::Vec3(0.0f, 0.0f, r)));
    geom->setVertexArray(v.get());

    //为每个顶点指定一种颜色，2个点颜色不一样时候，会产生渐变颜色
    osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array();
    c->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); //坐标原点为白色
    c->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)); //x red

    c->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); //坐标原点为白色
    c->push_back(osg::Vec4(0.8f, 0.5f, 0.88f, 1.0f)); //y green

    c->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); //坐标原点为白色
    c->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f)); //z blue

    c->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); //坐标原点为白色
    c->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)); // blue

    //如果没指定颜色则会变为黑色
    geom->setColorArray(c.get());
    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    //添加线段
    //geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4)); //XZ
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2)); //X
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 2, 2)); //Y
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 4, 2)); //Z
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 6, 2));

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,
            osg::StateAttribute::OFF);
    geode->addDrawable(geom.get());

    osg::ref_ptr<osg::Group> newroot = new osg::Group();
    newroot->addChild(geode.get());

    return newroot;
}
////////////////////////////////////////////////////////////////////////////////

void Satellite::initialize(int stage)
{
    bShowCone = false;


    // 加载2行数据配置文件
    std::string filepath = OsgEarthScene::getInstance()->par("tlePath").stringValue();
    tlefile.ParseFile(filepath);

    switch (stage) {
    case 0: {
        orbitTail = NULL;
        modelURL = par("modelURL").stringValue();
        modelScale = par("modelScale");
        labelColor = par("labelColor").stringValue();
        altitude = par("altitude");
        name1 = par("name").stringValue();
        taillength = par("taillength").intValue();
        orbitId = par("orbitId").stringValue();



        iNum = par("iNum").intValue();
        if (iNum == -1)
        {
            iNum = this->getIndex();
        }

        // 解析计算的开始时间
        std::string strTime = OsgEarthScene::getInstance()->par("targetTime").stringValue();
        time_t tmTarget = StringToDatetime(strTime);


        // 计算背景图大小
        //cCanvas *canvas = getParentModule()->getCanvas();
        const char * pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 0);
        this->width = atoi(pStr);
        pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 1);
        this->height = atoi(pStr);


        std::string str0, str1, str2;
        Satellite::getInfo(iNum, str0, str1, str2);

        std::cout << "index=" << iNum << ", name=" << str0 << std::endl;
        std::cout << str1 << std::endl;
        std::cout << str2 << std::endl;

        std::time_t timestamp = std::time(nullptr);       // get current time as an integral value holding the num of secs
                                                          // since 00:00, Jan 1 1970 UTC
        std::tm* currentTime = std::gmtime(&timestamp);   // convert timestamp into structure holding a calendar date and time


        currentTime = std::gmtime(&tmTarget);             // 不用当前时间，而用指定的时间
        // 设置当前时间，这里是从当前时间开始仿真，其实可以从配置文件读一个时间值，从某个时间开始算
        norad.setJulian(currentTime);
        // 初始化 仿真计时为0
        nextChange = SimTime();
        norad.initializeMobility(nextChange, str0, str1, str2);

        // ShowInfo();

        isWorking = true;


        // 初始化卫星位置
        simtime_t t;

        this->startingPos = this->pos = getPositionAtTime(t);

        // 这里矫正地球位置，不知道算法，根据卫星的经度，矫正一下地球角度
        if (this->getIndex() == 0)
        {
            osg::Vec3d temp = WorldToLonLatAlt(this->pos);

            double x2 = this->longitude > 0 ? longitude : longitude + 360;
            double x1 = temp.x() > 0 ? temp.x() :temp.x() + 360;
            double delta = x1 - x2;  // 当前天球角度 - 本来的角度
            OsgEarthScene::getInstance()->setGap(delta);
        }

        label = norad.getName1();

        // 这句是防止星星太多，都在同一个轨道配置中，线太多
        if (orbitId == "1")
        {
             orbitId = label;
        }
        getDisplayString().setTagArg("t", 0, label.c_str());

        getOsgCanvas()->setScene(osgDB::readNodeFile(modelURL));
        ////////////////////////////////////////////////////////////////////////
        // 二维
        cCanvas *canvas = getParentModule()->getCanvas();
        // 计算轨迹，使用拖尾方式
        try {
            cFigure *pFigure = canvas->getFigure("GEO"); // this->orbitId.c_str()
            if (pFigure) {
                cPolylineFigure *p = check_and_cast<cPolylineFigure*>(pFigure);
                //orbitTail = p;
                orbitTail = p->dup();
                canvas->addFigure(orbitTail);


            } else
                orbitTail = NULL;
        } catch (cException e) {
            std::cout << e.getFormattedMessage() << std::endl;
            orbitTail = NULL;
        }

        break;
    }
    case 1:
        ChannelController::getInstance()->addSatellite(this);

        auto scene = OsgEarthScene::getInstance()->getScene(); // scene is initialized in stage 0 so we have to do our init in stage 1
        mapNode = osgEarth::MapNode::findMapNode(scene);

        // build up the node representing this module
        // a GeoTransform allows positioning a model using world coordinates
        geoTransform = new osgEarth::GeoTransform();

        auto modelNode = osgDB::readNodeFile(modelURL);

        modelNode->getOrCreateStateSet()->setAttributeAndModes(
          new osg::Program(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        modelNode->getOrCreateStateSet()->setMode(
                  GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

        // scale and rotate the model if necessary
        auto pat = new osg::PositionAttitudeTransform();
        this->pat = pat;
        pat->setScale(osg::Vec3d(modelScale, modelScale, modelScale));

        auto objectNode = new cObjectOsgNode(this);
        pat->addChild(objectNode);
        objectNode->addChild(modelNode);
        geoTransform->addChild(pat);

        // 设置卫星的名字
        // set the name label if the color is specified
        if (!labelColor.empty()) {
            //Style
            labelStyle.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_CENTER_TOP;
            labelStyle.getOrCreate<TextSymbol>()->declutter() = true;
            labelStyle.getOrCreate<TextSymbol>()->pixelOffset() = osg::Vec2s(0,50);
            labelStyle.getOrCreate<TextSymbol>()->fill()->color() = osgEarth::Color(labelColor);
            labelStyle.getOrCreate<TextSymbol>()->encoding() = TextSymbol::Encoding::ENCODING_UTF8;
            labelStyle.getOrCreate<TextSymbol>()->font() = "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径


            labelStyledown.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_CENTER_TOP;
            labelStyledown.getOrCreate<TextSymbol>()->declutter() = true;
            labelStyledown.getOrCreate<TextSymbol>()->pixelOffset() = osg::Vec2s(0,50);
            labelStyledown.getOrCreate<TextSymbol>()->fill()->color() = osgEarth::Color("#ff0000ff");
            labelStyledown.getOrCreate<TextSymbol>()->encoding() = TextSymbol::Encoding::ENCODING_UTF8;
            labelStyledown.getOrCreate<TextSymbol>()->font() = "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径

            //geoTransform->addChild(new LabelNode(getFullName(), labelStyle));
            // 设置卫星的显示名字
            //label += "test";
            labelNode = new LabelNode(label.c_str(), labelStyle);
            geoTransform->addChild(labelNode);
        }

        // add the locator node to the scene
        scene->asGroup()->addChild(geoTransform);

        // making the orbit circle
        // 轨道的圆圈
        std::string orbitColor = par("orbitColor");
        if (!orbitColor.empty()) {
            osg::ref_ptr<osg::Geometry> orbitGeom = new osg::Geometry;
            osg::ref_ptr<osg::DrawArrays> drawArrayLines = new  osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP);
            osg::ref_ptr<osg::Vec3Array> vertexData = new osg::Vec3Array;

            orbitGeom->addPrimitiveSet(drawArrayLines);
            auto stateSet = orbitGeom->getOrCreateStateSet();
            stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateSet->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
            stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            stateSet->setAttributeAndModes(new osg::LineWidth(1.5), osg::StateAttribute::ON);
            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            auto depth = new osg::Depth;
            depth->setWriteMask(false);
            stateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);

            // 这里是计算了圆形的各个点，然后连线， 形成了一个圆
            // seconds
            double period = norad.getPeriod();

            for (int i = 0; i <= 100; ++i)
            {
                simtime_t t = SimTime(period * i / 100.0);
                simtime_t now;
                now += t;

                vertexData->push_back(getPositionAtTime( now ) ) ;
            }

            // 把各个点串成一个线
            orbitGeom->setVertexArray(vertexData);
            drawArrayLines->setFirst(0);
            drawArrayLines->setCount(vertexData->size());

            // 设置所有线的颜色都是使用这个
            osg::ref_ptr<osg::Vec4Array> colorData = new osg::Vec4Array;
            colorData->push_back(osgEarth::Color(orbitColor));
            orbitGeom->setColorArray(colorData, osg::Array::BIND_OVERALL);

            //
            osg::ref_ptr<osg::Geode> orbitGeode = new osg::Geode;
            orbitGeode->addDrawable(orbitGeom.get());

            // 将轨道对象，添加到场景中去
            scene->asGroup()->addChild(orbitGeode);
        }

        // 绘制坐标系
        scene->asGroup()->addChild(makeCoordinate());

        // 射程圆锥的网状绘图
        std::string coneColor = par("coneColor");
        if (!coneColor.empty() && bShowCone)
        {
            double orbitRadius = earthRadius + altitude; // in kilometers
            // the angle between the center of the earth and the horizon as seen from the satellite, in radians
            double alpha = std::asin(earthRadius / orbitRadius);
            // the distance of the horizon from the satellite, in meters
            double horizonDistance = std::sqrt(orbitRadius * orbitRadius - earthRadius * earthRadius) * 1000;
            double coneHeight = std::sin(alpha)*horizonDistance;
            double coneRadius = std::cos(alpha)*horizonDistance;
            // the offset is to position the tip to the satellite
            osg::Cone *cone = new osg::Cone(osg::Vec3(0, 0, -coneRadius*0.75), coneHeight, coneRadius);

            osg::ref_ptr<osg::Geode> coneGeode = new osg::Geode;
            auto coneDrawable = new osg::ShapeDrawable(cone);
            coneDrawable->setColor(osgEarth::Color(coneColor));

            coneGeode->addDrawable(coneDrawable);
            coneGeode->getOrCreateStateSet()->setAttribute(new osg::PolygonMode(
                osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE));
            coneGeode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            auto depth = new osg::Depth;
            depth->setWriteMask(false);
            coneGeode->getOrCreateStateSet()->setAttributeAndModes(depth, osg::StateAttribute::ON);
            geoTransform->addChild(coneGeode);
        }


    }// end of switch
}


// 计算仰角和方位时候需要提供位置信息
cEci & Satellite::getPositionSdp4()
{
    return norad.getEci();
}

int Satellite::setStationInsight(int i)
{
    this->numStationInsight = i;
    return 1;
}
void Satellite::setAngle(double n)
{
    this->angle = n;
}
int Satellite::getNumStationInsight()
{
    return this->numStationInsight;
}

// 未来某个时间设置后，得到当前的位置信息
osg::Vec3d Satellite::getPositionAtTime(simtime_t& targetTime)
{
    double  x= 0, y= 0, z= 0;
    norad.updateTime(targetTime);

    norad.getXYZ(x , y, z);

    x = x  * 1000;
    y = y * 1000;
    z = z * 1000;

  //  osg::Vec3d xyz = osg::Vec3d(x, y, z);

    // 这里计算后的海拔是千米
    norad.getLogLatAlt(longitude, latitude, altitude);
    altitude = altitude * 1000;

    //std::cout << "xyz:" << x  << ", " << y  << ", " << z << "," << pos.length() << std::endl;
    //std::cout << "经纬:" << longitude << ", " <<  latitude  << ", " <<  altitude << std::endl;
    return osg::Vec3d(x, y, z);
}

// 显示调试信息
void Satellite::ShowInfo()
{
    std::cout << "半长轴：" << norad.getSemiMajor()        << std::endl;
    std::cout << "偏心率：" << norad.getEccentricity()     << std::endl;
    std::cout << "轨道倾角：" << norad.getInclination()    << std::endl;
    std::cout << "升交点：" << norad.getRAAN()             << std::endl;
    std::cout << "近地点幅角：" << norad.getArgPerigee()    << std::endl;
    std::cout << "平近角：" << norad.getMeanAnomaly()       << std::endl;
    std::cout << "周期（秒）：" << norad.getPeriod()        << std::endl;
}

void Satellite::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

double Satellite::getDeltaLongitude(double x)
{
    // 计算地球转过的角度
    // 按秒计算当前转过的角度，

            double angleEarth = simTime().inUnit(SIMTIME_S) * 360  / ((23.0 * 60 + 56) *60 + 4.0) ;
            angleEarth = fmod(angleEarth, 360);

            double temp = x;
            if (temp < 0)
                temp += 360;

            temp =  temp - angleEarth;
            if (temp > 180)
                temp = temp -360;

            if (temp < -180)
            {
                temp += 360;
            }

            return temp;
}


// 用时间乘平均 角速度，得到当前角度
// 通过角度，计算当前位置
void Satellite::updatePosition()
{
    //double t = simTime().inUnit(SIMTIME_MS) / 1000.0;  ?????
    //std::cout << "更新位置" << endl;
    simtime_t t = simTime();
    if (t != lastPositionUpdateTime)
    {

        // 得到当前位置
        this->pos = getPositionAtTime(t);

        osg::Vec3d v;
        //osg::Vec3d v = osg::Vec3d(110.5, 0, 35789000);

        // 得到空间参考，椭球面信息，将世界坐标转为经纬度？所以应该避免这样的变量命名方式！！！
        mapNode->getMapSRS()->transformFromWorld(pos, v);
        geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS(), v));

        lastPositionUpdateTime = t;
    }


    std::string str;
    // 三维空间设置开机关机代码
    if (isStatusChanged)
    {
        labelNode->setDynamic(true);

        if (isWorking == false)
        {
            str = label + " 宕机";
           labelNode->setStyle(labelStyledown);
        }
        else
        {
            str = label;
            labelNode->setStyle(labelStyle);

        }

        labelNode->setText(str.c_str());


        isStatusChanged = false;
    }
    /////////////////////////////////////////////////////////////////////
    // 二维的
    if (longitude > 180.0)
    {
        longitude = longitude - 360;
    }
    platx = width / 2 + longitude / 360.0 * width,
    platy = height / 2 - latitude / 180.0 * height;

    tooltip = "";
    std_string_format(tooltip, "经度：%lf, 纬度：%lf, 仰角：%lf", longitude, latitude, angle);

    if (orbitTail != NULL) {
           // 拖尾中第一个点
           if (orbitTail->getNumPoints() == 0) {
               last_x = platx;
           }

           // 符号改变
           if (((last_x < platx) && abs(platx - last_x) > width /2)
                   || ((last_x > platx) && abs(last_x - platx) > width /2)) {
               while (orbitTail->getNumPoints() > 0)
                   orbitTail->removePoint(0);
           }
           cFigure::Point pt = cFigure::Point(platx, platy);
           orbitTail->addPoint(pt);
           if (orbitTail->getNumPoints() > taillength)
               orbitTail->removePoint(0);
       }

       last_x = platx;

       // 区分拖尾颜色
       if (altitude < 35000 * 1000)
             orbitTail->setLineColor(omnetpp::cAbstractImageFigure::MAGENTA);

}


void Satellite::refreshDisplay() const
{
    const_cast<Satellite *>(this)->updatePosition();

    getDisplayString().setTagArg("p", 0, platx);
    getDisplayString().setTagArg("p", 1, platy);

    getDisplayString().setTagArg("tt", 0, tooltip.c_str());
    if (orbitId == "M1")
    {
        getDisplayString().setTagArg("t", 2, "#ff9406");
    }
    else if (orbitId == "M2")
    {
        getDisplayString().setTagArg("t", 2, "#03ee00");
    }
    else
    {
        getDisplayString().setTagArg("t", 2, "#2200cd");
    }

    if (this->numStationInsight > 0)
    {
        //getDisplayString().setTagArg("i2", 0, "device/card_vs");
        if (orbitId == "M1")
            getDisplayString().setTagArg("b", 4, "#ff9406");
        else if (orbitId == "M2")
            getDisplayString().setTagArg("b", 4, "#03ee00");
        else
            getDisplayString().setTagArg("b", 4, "#2200cd");

        getDisplayString().setTagArg("b", 5, 2);
    }
    else
    {
        getDisplayString().removeTag("b");
        //getDisplayString().setTagArg("b", 4, "#000000");
        //getDisplayString().setTagArg("b", 5, (int)0);
    }
}

#endif // WITH_OSG


