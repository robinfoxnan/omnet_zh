//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#if defined(WITH_OSG) && defined(WITH_OSGEARTH)

#include "ChannelController.h"

#include <osgEarthUtil/LinearLineOfSight>
#include <osgUtil/UpdateVisitor>
#include <osg/ValueObject>
#include <osg/LineWidth>
#include <osg/Depth>
#include <osg/vec3d>

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;

Define_Module(ChannelController);

ChannelController *ChannelController::instance = nullptr;

template<typename T>
bool contains(const std::vector<T>& vector, T item) { return std::find(vector.begin(), vector.end(), item) != vector.end(); }


/*
 * 两个向量的夹角计算公式：θ=acos(v1⋅v2/||v1||||v2||)
 * 需要注意的θ的范围再0~pi之间，怎么解决0~2pi的问题呢？
 * 这里就需要用到向量的叉乘，根据叉乘后的向量与屏幕方向来确定角度是否超过180°。
 */
double AngleLine2(osg::Vec3d line1, osg::Vec3d line2)  // , osg::Vec3d direction
{
    line1.normalize();
    line2.normalize();
    double temp = (line1 * line2);
    // 1和-1时为0和180°
    if (fabs(fabs(temp) - 1) < DBL_EPSILON)
    {
        return temp > 0 ? 0 : osg::PI;
    }
    double angle = (double)acos(temp);

    angle = angle / osg::PI * 180;

    // 两个向量的叉乘结果与屏幕方向是否一致来判断角度是否超过180°
    //osg::Vec3f axis = line1 ^ line2;
    // return axis * direction > 0 ? angle : 2 * osg::PI - angle;
    // 我们计算夹角不需要考虑
    return angle;

}

// 计算point1的角度观察，point2---point1---地心角，是否够大？
bool isInAngleSight(osg::Vec3d point1, osg::Vec3d point2)
{
    osg::Vec3d point0 = osg::Vec3d(0.0, 0.0, 0.0);
    osg::Vec3d line1 = point0 - point1;
    osg::Vec3d line2 = point2 - point1;

    double angle = AngleLine2(line1, line2);

    // 这里设置一个卫星天线的可视角度，这里是一半
    if (angle < 50.0)
        return true;


    return false;

}

ChannelController::ChannelController()
{
    if (instance)
        throw cRuntimeError("There can be only one ChannelController instance in the network");
    instance = this;
}

ChannelController::~ChannelController()
{
    instance = nullptr;
}

ChannelController *ChannelController::getInstance()
{
    if (!instance)
        throw cRuntimeError("ChannelController::getInstance(): there is no ChannelController module in the network");
    return instance;
}

void ChannelController::initialize(int stage)
{
    switch (stage) {
    case 0:
        satToSatColor = par("satToSatColor").stringValue();
        satToSatWidth = par("satToSatWidth").doubleValue();
        satToGroundColor = par("satToGroundColor").stringValue();
        satToGroundWidth = par("satToGroundWidth").doubleValue();
        break;
    case 1:
        scene = OsgEarthScene::getInstance()->getScene()->asGroup();
        connections = new osg::Geode();
        scene->addChild(connections);
        break;
    case 2:
        // note: satellites and ground stations must have been added by now
        for (int i = 0; i < (int)satellites.size(); ++i)
        {
            //if (satellites[i]->isWorking == false)
           //     continue;

            for (int j = 0; j < (int)stations.size(); ++j)
            {
                addLineOfSight(satellites[i]->getLocatorNode(), stations[j]->getLocatorNode(), 0, i, -1);
            }

            for (int j = i+1; j < (int)satellites.size(); ++j)
            {
                // 同轨道号比较
                if (satellites[i]->getOrbitId() == satellites[j]->getOrbitId())
                {
                    /*
                    int ki = (int)satellites[i]->getStartingPhase();

                    int kj = (int)satellites[j]->getStartingPhase();


                    // kj 跨越360 加80 追上ki
                    int delta2 = std::abs( ((kj + 80) % 360 ) - ki);

                    // kj 比ki大80度
                    int delta1 = std::abs(ki + 80  - kj);

                    if ((delta1 < 3) || (delta2 < 3))
                        addLineOfSight(satellites[i]->getLocatorNode(), satellites[j]->getLocatorNode(), 1);
                        */
                    //bool isWorking = satellites[i]->isWorking && satellites[j]->isWorking;   // 是否在工作
                    bool ret = isInAngleSight(satellites[i]->getPos(), satellites[j]->getPos());
                    if (ret)
                        addLineOfSight(satellites[i]->getLocatorNode(), satellites[j]->getLocatorNode(), 1, i, j);
                }
            }
        }
        break;
    }
}


void ChannelController::addSatellite(Satellite *p)
{
    ASSERT(!contains(satellites, p));
    ASSERT(losNodes.empty()); // we are before init stage 2
    satellites.push_back(p);
}

void ChannelController::addGroundStation(GroundStation *p)
{
    ASSERT(!contains(stations, p));
    ASSERT(losNodes.empty()); // we are before init stage 2
    stations.push_back(p);
}

void ChannelController::addLineOfSight(osg::Node *a, osg::Node *b, int type, int num1, int num2)
{
    auto mapNode = osgEarth::MapNode::findMapNode(scene);
    osgEarth::Util::LinearLineOfSightNode *los = new osgEarth::Util::LinearLineOfSightNode(mapNode);
    losNodes.push_back(los);

    // not drawing the line of sight nodes' lines
    los->setGoodColor(osg::Vec4f(0, 0, 0, 0));
    los->setBadColor(osg::Vec4f(0, 0, 0, 0));

    auto stateSet = los->getOrCreateStateSet();
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto depth = new osg::Depth;
    depth->setWriteMask(false);
    stateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);

    los->setUserValue("type", type);

    // 添加两个节点
    los->setUserValue("index1", num1);
    los->setUserValue("index2", num2);

    los->setUpdateCallback(new osgEarth::Util::LineOfSightTether(a, b));
    los->setTerrainOnly(true); // so the dish model itself won't occlude

    scene->addChild(los);
}

void ChannelController::refreshDisplay() const
{
    // make sure satellite positions are up to date
    for (auto satellite : satellites)
        satellite->updatePosition();

    // los update callbacks are called during update traversal, so do it now
    osgUtil::UpdateVisitor updateVisitor;
    scene->traverse(updateVisitor);

    // update line-of-sight lines (remove all, then add back current ones)
    connections->removeDrawables(0, connections->getNumDrawables());

    int numSatToSat = 0;
    int numSatToGround = 0;

    bool isworking = true;

    for (auto losNode : losNodes) {
        if (losNode->getHasLOS()) {
            auto start = losNode->getStartWorld();
            auto end = losNode->getEndWorld();

            int type;
            losNode->getUserValue("type", type);

            // 为了测试某个卫星是否在工作
            int num1, num2;
            losNode->getUserValue("index1", num1);
            losNode->getUserValue("index2", num2);

            isworking = true;
            if (num1 > -1)
            {
                isworking =  isworking && satellites[num1]->isWorking;
            }

            if (num2 > -1)
            {
                isworking =  isworking && satellites[num2]->isWorking;
            }

            switch (type) {
            case 0:
                ++numSatToGround;
                if (!satToGroundColor.empty() && isworking)
                    connections->addDrawable(createLineBetweenPoints(start, end, satToGroundWidth, osgEarth::Color(satToGroundColor)));
                break;
            case 1:
                ++numSatToSat;
                if (!satToSatColor.empty() && isworking)
                    connections->addDrawable(createLineBetweenPoints(start, end, satToSatWidth, osgEarth::Color(satToSatColor)));
                break;
            }
        }
    }

    EV << "Active connections: " << numSatToSat << " sat-to-sat and " << numSatToGround << " sat-to-ground \n";
}

osg::ref_ptr<osg::Drawable> ChannelController::createLineBetweenPoints(osg::Vec3 start, osg::Vec3 end, float width, osg::Vec4 color)
{
    osg::ref_ptr<osg::Geometry> orbitGeom = new osg::Geometry;
    osg::ref_ptr<osg::DrawArrays> drawArrayLines = new  osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP);
    osg::ref_ptr<osg::Vec3Array> vertexData = new osg::Vec3Array;

    orbitGeom->addPrimitiveSet(drawArrayLines);
    auto stateSet = orbitGeom->getOrCreateStateSet();
    stateSet->setAttributeAndModes(new osg::LineWidth(width), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto depth = new osg::Depth;
    depth->setWriteMask(false);
    stateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);

    vertexData->push_back(start);
    vertexData->push_back(end);

    orbitGeom->setVertexArray(vertexData);
    drawArrayLines->setFirst(0);
    drawArrayLines->setCount(vertexData->size());

    osg::ref_ptr<osg::Vec4Array> colorData = new osg::Vec4Array;
    colorData->push_back(osgEarth::Color(color));
    orbitGeom->setColorArray(colorData, osg::Array::BIND_OVERALL);
    return orbitGeom;
}


void ChannelController::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

std::string ChannelController::getLineColor(std::string name)
{
    if (name == "MEO1")
    {
        return "#00FF00BF";
    }



    return "#00FF00BF";
}

#endif // WITH_OSG
