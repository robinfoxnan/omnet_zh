//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#if defined(WITH_OSG) && defined(WITH_OSGEARTH)

#include "GroundStation.h"
#include "OsgEarthScene.h"
#include "ChannelController.h"
#include "omnetpp/osgutil.h"

#include <osg/Node>
#include <osg/Texture>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/PositionAttitudeTransform>
#include <osgEarth/Capabilities>
#include <osgEarthAnnotation/LabelNode>
#include <osgEarthSymbology/Geometry>
#include <osgEarthFeatures/Feature>

using namespace omnetpp;

Define_Module(GroundStation)

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Features;


void GroundStation::initialize(int stage)
{
    switch (stage) {
    case 0: {
        timeStep = par("timeStep");
        modelURL = par("modelURL").stringValue();
        modelScale = par("modelScale");
        labelColor = par("labelColor").stringValue();
        longitude = par("longitude");
        latitude = par("latitude");

        // 计算背景大小
        //cCanvas *canvas = getParentModule()->getCanvas();
        const char * pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 0);
        this->width = atoi(pStr);
        pStr = this->getParentModule()->getDisplayString().getTagArg("bgb", 1);
        this->height = atoi(pStr);

        getOsgCanvas()->setScene(osgDB::readNodeFile(modelURL));
        break;
    }
    case 1:
        ChannelController::getInstance()->addGroundStation(this);

        auto scene = OsgEarthScene::getInstance()->getScene(); // scene is initialized in stage 0 so we have to do our init in stage 1
        mapNode = osgEarth::MapNode::findMapNode(scene);

        // build up the node representing this module
        // a GeoTransform allows positioning a model using world coordinates
        geoTransform = new osgEarth::GeoTransform();

        auto modelNode = osgDB::readNodeFile(modelURL);

        geoTransform->getOrCreateStateSet()->setAttributeAndModes(
          new osg::Program(), osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
        geoTransform->getOrCreateStateSet()->setMode(
                          GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

        // scale and rotate the model if necessary
        auto pat = new osg::PositionAttitudeTransform();
        pat->setScale(osg::Vec3d(modelScale, modelScale, modelScale));

        auto objectNode = new cObjectOsgNode(this);
        pat->addChild(objectNode);
        objectNode->addChild(modelNode);
        geoTransform->addChild(pat);

        // set the name label if the color is specified
        if (!labelColor.empty()) {
            Style labelStyle;
            labelStyle.getOrCreate<TextSymbol>()->alignment() = TextSymbol::ALIGN_CENTER_TOP;
            labelStyle.getOrCreate<TextSymbol>()->declutter() = true;
            labelStyle.getOrCreate<TextSymbol>()->pixelOffset() = osg::Vec2s(0,40);
            labelStyle.getOrCreate<TextSymbol>()->fill()->color() = osgEarth::Color(labelColor);
            labelStyle.getOrCreate<TextSymbol>()->encoding() = TextSymbol::Encoding::ENCODING_UTF8;
            labelStyle.getOrCreate<TextSymbol>()->font() = "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径
            geoTransform->addChild(new LabelNode(par("label"), labelStyle));
        }

        // add the locator node to the scene
        mapNode->addChild(geoTransform);

        // position the nodes, so we will see them at correct position right after initialization
        refreshVisuals();

        break;
    }
}

void GroundStation::refreshVisuals()
{
    geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS(), longitude, latitude, altitude));


    // 这里第度数是按照360度来计算的，不是按照pi计算
    int x = width / 2.0 + longitude / 180.0 * width / 2.0;
    int y = height / 2.0 - latitude  / 90.0 * height / 2.0;

    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);
}

void GroundStation::handleMessage(cMessage *msg)
{

}

#endif // WITH_OSG
