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
//#include <osg/Vec3f>

using namespace omnetpp;

Define_Module(Satellite)

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;



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

    switch (stage) {
    case 0: {
        modelURL = par("modelURL").stringValue();
        modelScale = par("modelScale");
        labelColor = par("labelColor").stringValue();
        altitude = par("altitude");
        name = par("name").stringValue();

        // 计算背景图大小
        //cCanvas *canvas = getParentModule()->getCanvas();
        const char * pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 0);
        this->width = atoi(pStr);
        pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 1);
        this->height = atoi(pStr);

        x0 = width / 2;
        y0 = height / 2;

        // 后加的
        ascendingLongitude = par("ascendingLongitude").doubleValue();
        planeAngle = par("planeAngle").doubleValue();

        startingPhaseDegree = par("startingPhase").doubleValue();
        phase = startingPhase = par("startingPhase").doubleValue() * M_PI / 180.0;

        std::string normalString = par("orbitNormal");
        orbitId = par("orbitId").stringValue();

        isWorking = true;

        // 兼容旧代码，之后删除
        if (normalString.empty()) {
            // 如果法线为空，则自己设置一个伪随机数
            // it is not a correct spherical distribution, nor deterministic, but will do here
             normal.set(dblrand() * 2 - 1, dblrand() * 2 - 1, dblrand() * 2 - 1);
        }
        else {
            std::stringstream ss(normalString);

            double x, y, z;
            ss >> x;
            ss.ignore();
            ss >> y;
            ss.ignore();
            ss >> z;

            if (!ss)
                throw cRuntimeError("Couldn't parse orbit normal vector \"%s\", the correct format is for example \"2.5,3,0\", or leave it empty for random", normalString.c_str());

            normal.set(x, y, z);

            // 如果是新添加的igso，新算一个normatl
            if (planeAngle > 0.00001)
            {
                normal = computeNormalForIGSO(altitude, planeAngle, ascendingLongitude);
            }

        }

        normal.normalize();

        auto c1 = normal ^ osg::Vec3d(0, 1, 0);  // 与Y轴做叉积，得到平面上垂直于Y的一条直线
        auto c2 = normal ^ osg::Vec3d(1, 0, 0);  // 与X轴做叉积，得到平面上垂直于X的一条直线
        osg::Vec3d &cross = c1.length2() < 0.1 ? c2 : c1;

        cross.normalize();

        // 在轨道平面上的，新的X轴和Y轴，这个时候法线就相当于Z轴
        orbitX = cross;
        orbitY = normal ^ cross;

        // 对GEO单独设置XY轴
        if ( planeAngle < 0.000001)
        {
            orbitX = osg::Vec3d(1, 0, 0);
            orbitY = osg::Vec3d(0, 1, 0);


        }
        else // 先测试一下IGSO，可以理解为从GEO在Z轴旋转一下，然后，然后升交点-地心-降交点连线就是新X轴
        {   // 初始时候是X, Y一样的
            // 首先是按照升交点赤经，在Z轴旋转一下轨道面,
            // 地心
            osg::Vec3d point0 = osg::Vec3d(0, 0, 0);

            // 升交点
            osg::Vec3d point1 = osg::Vec3d(ascendingLongitude, 0, altitude * 1000);
            osg::Vec3d point1XYZ =  LonLatAltToWorld(point1);

            // 高点, i就是倾角，也就是纬度
            osg::Vec3d point2 = osg::Vec3d(ascendingLongitude + 90, planeAngle, altitude * 1000);
            osg::Vec3d point2XYZ =  LonLatAltToWorld(point2);


            orbitX = point1XYZ - point0;
            orbitY = point2XYZ - point0;
            orbitX.normalize();
            orbitY.normalize();

        }
        // 初始化卫星位置
        this->startingPos = getPositionAtPhase(startingPhase);

        if (this->orbitId.find("GEO") != std::string::npos)
        {
            // 设置图标
            // *.sat[0].@display="i=device/satellite";
            getDisplayString().setTagArg("i", 0, "device/satellite");
            this->setName("静止");
        }

        else if (this->orbitId.find("IGSO") != std::string::npos)
        {

            getDisplayString().setTagArg("i", 0, "device/satellit_yellow");
            this->setName("同步倾斜");
        }

        else if (this->orbitId.find("MEO1") != std::string::npos)
        {
             // 设置图标
             getDisplayString().setTagArg("i", 0, "device/satellit_red");
             this->setName("中轨");
        }
        else if (this->orbitId.find("MEO2") != std::string::npos)
        {
             // 设置图标
             getDisplayString().setTagArg("i", 0, "device/satellit_purple");
             this->setName("中轨");
         }
        else if (this->orbitId.find("MEO3") != std::string::npos)
        {
             // 设置图标
             getDisplayString().setTagArg("i", 0, "device/satellit_blue");
             this->setName("中轨");
        }

        // 2d显示名字
        label = this->orbitId + "-" + name;
        getDisplayString().setTagArg("t", 0, label.c_str());

        //std::cout << getDisplayString().str() << std::endl;

        getOsgCanvas()->setScene(osgDB::readNodeFile(modelURL));

        cCanvas *canvas = getParentModule()->getCanvas();
        // 计算轨迹，使用拖尾方式
        try{

            //std::string text = this->orbitId + name;
           cFigure * pFigure =  canvas->getFigure(label.c_str());  // this->orbitId.c_str()
           if (pFigure)
           {
               cPolylineFigure * p = check_and_cast<cPolylineFigure *>(pFigure);
               orbitTail = p;
               //p->dup();
               //canvas->addFigure(orbitTail);
               //orbitTail->setLineColor(omnetpp::cAbstractImageFigure::RED);

           }
           else
               orbitTail = NULL;
        }
        catch(cException e)
        {
              std::cout << e.getFormattedMessage() << std::endl;
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

            // 这里是计算了圆形的各个点，然后连线，形成了一个圆
            for (int i = 0; i <= 100; ++i)
                vertexData->push_back(getPositionAtPhase(i / 100.0 * M_PI*2));

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

    simtime_t t = simTime();
    if (t != lastPositionUpdateTime) {
        phase = startingPhase + t.dbl() * getOmega();  //FIXME getOmega(); ???
        pos = getPositionAtPhase(phase);

        osg::Vec3d v;

        // 得到空间参考，椭球面信息，将世界坐标转为经纬度？所以应该避免这样的变量命名方式！！！
        mapNode->getMapSRS()->transformFromWorld(pos, v);

        std::cout << "经纬度：" <<  v.x() << ", " << v.y()  << ", "<< v.z() << endl;
        geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS(), v));

        lastPositionUpdateTime = t;

        double temp = getDeltaLongitude(v.x());

        // 计算星下点位置
        // 这里第度数是按照360度来计算的，不是按照pi计算
        // 对GEO,IGSO单独设置，按照高度来计算

        if ( abs(35786 - this->altitude) < 1)
        {
            if (planeAngle < 0.1)  //geo
            {
                x = x0 + startingPhaseDegree / 180.0 * x0 ;
            }
            else  // igso
            {/*
                // 这样算的不对，之所以出现8字，是因为轨道有倾角，对赤道面投影不是均匀的。
                if (ascendingLongitude> 180)
                {
                    x = x0 + (ascendingLongitude - 360) / 180.0 * x0 ;
                }
                else
                {
                   x = x0 + ascendingLongitude / 180.0 * x0 ;

                }
                */

                x = x0 + temp / 180.0 * x0 ;

            }

        }
        else   // MEO
        {
            x = x0 + temp / 180.0 * x0 ;

        }
        y = y0 - v.y()  / 90.0 * y0;



        if (orbitTail != NULL)
        {
            if (orbitTail->getNumPoints() == 0)
            {
                        last_x = x;
            }

            // 符号改变
            if (
                    ((last_x < x) && abs(x - last_x) > x0 )  ||
                    ((last_x > x) && abs(last_x - x) > x0 )
                    )
            {
                while(orbitTail->getNumPoints() > 0)
                    orbitTail->removePoint(0);
            }
            cFigure::Point pt = cFigure::Point(x, y);
                orbitTail->addPoint(pt);
            if (orbitTail->getNumPoints() > 1000)
                orbitTail->removePoint(0);
        }

        last_x = x;
    }

    std::string str;

    if (isStatusChanged)
    {
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
        labelNode->setDynamic(true);
        labelNode->setText(str.c_str());


        isStatusChanged = false;
    }


}

// 一个圆周是弧度是：2 * pi，每次去1/100，调用这个函数计算，计算出100个点，就是一个圆周的采样点
// 用半径直接乘一下当前卫星已经走过的角度
// cos(i)是当前X分量，sin(i)是当前的Y分量，2个向量相加，重新得到当前单位向量；乘半径就是向量
osg::Vec3 Satellite::getPositionAtPhase(double alpha) const
{
    return (orbitX * std::cos(alpha) + orbitY * std::sin(alpha)) * (earthRadius + altitude) * 1000;
}

void Satellite::refreshDisplay() const
{
    const_cast<Satellite *>(this)->updatePosition();

    // update the position on the 2D canvas
    // 2d 的显示信息
    //std::string label = this->orbitId + "-" + name;
    //getDisplayString().setTagArg("t", 0, label.c_str());
    //getDisplayString().setTagArg("p", 0, 300 + pos.x() / 100000);
    //getDisplayString().setTagArg("p", 1, 300 - pos.y() / 100000);

    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);


}


#endif // WITH_OSG
