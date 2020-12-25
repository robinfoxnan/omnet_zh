//==========================================================================
//  modulelayouter.cc - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "modulelayouter.h"

#include <omnetpp/cdisplaystring.h>
#include <QMessageBox>
#include <QPushButton>
#include "qtenv.h"
#include "layout/graphlayouter.h"
#include "layout/basicspringembedderlayout.h"
#include "layout/forcedirectedgraphlayouter.h"
#include "layouterenv.h"
#include "mainwindow.h"
#include "submoduleitem.h"  // for DEFAULT_ICON

#define emit

namespace omnetpp {
using namespace layout;
namespace qtenv {

class InteractiveTimeoutBasicGraphLayouterEnvironment : public BasicGraphLayouterEnvironment {

    bool okToProceed() override {
        if (BasicGraphLayouterEnvironment::okToProceed())
            return true;
        else {
            QMessageBox box(getQtenv()->getMainWindow());

            box.setIcon(QMessageBox::Question);
            box.setWindowTitle("布局好像比较慢啊");
            box.setText("布局子模块位置为平时慢.\n你要加速么?");

            // The actual roles don't matter to us, but on different platforms they might be placed/ordered differently
            QPushButton *stopButton = box.addButton("停止", QMessageBox::ButtonRole::NoRole);
            QPushButton *waitButton = box.addButton("继续等待", QMessageBox::ButtonRole::YesRole);
            QPushButton *goButton = box.addButton("运行", QMessageBox::ButtonRole::AcceptRole);

            box.exec();
            QAbstractButton *clickedButton = box.clickedButton();

            if (clickedButton == waitButton) {
                restartTimeout();
                return true;
            } else if (clickedButton == stopButton) {
                return false;
            } else if (clickedButton == goButton) {
                setTimeout(0);
                return true;
            } else
                return false;
        }
    }

};

// 这个函数主要是根据NED中描述的子模块位置确定一个在父模块中的位置
// b[0..5] = width, height, shape, fill color, border color, border width
// 关于p定位的：行、列、矩阵、环形、精确布局
void ModuleLayouter::getSubmoduleCoords(cModule *submod, bool &explicitcoords, 
                bool &obeysLayout, double &x, double &y, double &sx, double &sy, 
                double zoomFactor, double imageSizeFactor)
{
    const cDisplayString blank;
    const cDisplayString& ds = submod->hasDisplayString() && submod->parametersFinalized() ? submod->getDisplayString() : blank;

    // get size -- we'll need to return that too, and may be needed for matrix, ring etc. layout
    // -1 means unspecified
    double shapesx = -1, shapesy = -1;
    double iconsx = -1, iconsy = -1;

    // getting the "shape" (box or ellipse) size
    if (ds.containsTag("b")) {
        shapesx = resolveDoubleDispStrArg(ds.getTagArg("b", 0), submod, -1);
        shapesy = resolveDoubleDispStrArg(ds.getTagArg("b", 1), submod, -1);

        // if only one is present, use it for both
        if (shapesy == -1)
            shapesy = shapesx;
        if (shapesx == -1)
            shapesx = shapesy;

        // if none is present, factory defaults
        if (shapesx == -1) {
            shapesx = 40;
            shapesy = 24;
        }
    }

    //TODO do it like in submoduleitem.cc

    // getting the "image" size
    if (ds.containsTag("i")) {
        const char *imgName = ds.getTagArg("i", 0);
        if (*imgName) {
            // will return the unknownImage if imgName not found, 前一句是写错了吧
            auto img = getQtenv()->icons.getImage(imgName, ds.getTagArg("is", 0)); 
            iconsx = img->width(); // note: no need to take image scaling factor and zoom level into account
            iconsy = img->height();
        }
    }
    else if (!ds.containsTag("b")) { // if no image nor shape size is specified, using the default submodule icon
        auto img = getQtenv()->icons.getImage(SubmoduleItem::DEFAULT_ICON, ds.getTagArg("is", 0));
        iconsx = img->width();
        iconsy = img->height();
    }

    // applying the scaling factors
    if (iconsx != -1) iconsx *= imageSizeFactor;
    if (iconsy != -1) iconsy *= imageSizeFactor;

    if (shapesx != -1) shapesx *= zoomFactor;
    if (shapesy != -1) shapesy *= zoomFactor;

    // if no shape, then "ignoring" it
    if (shapesx == -1) { // boxsx and boxsy are both defined or both undefined.
        shapesx = iconsx;
        shapesy = iconsy;
    }

    sx = std::max(iconsx, shapesx);
    sy = std::max(iconsy, shapesy);

    // first, see if there's an explicit position ("p=" tag) given
    x = resolveDoubleDispStrArg(ds.getTagArg("p", 0), submod, -1);
    y = resolveDoubleDispStrArg(ds.getTagArg("p", 1), submod, -1);
    explicitcoords = x != -1 && y != -1;

    // set missing coordinates to zero
    if (x == -1)
        x = 0;
    if (y == -1)
        y = 0;

    // 查看用户设置的自动布局方式，r c m ri x
    const char *layout = ds.getTagArg("p", 2);  // matrix, row, column, ring, exact etc.
    obeysLayout = (layout && *layout);

    // modify x,y using predefined layouts
    // 是NULL, 或者""，就啥也不做了，那么？？
    
    if (!layout || !*layout) {
        // we're happy
    }
    else if (!strcmp(layout, "e") || !strcmp(layout, "x") || !strcmp(layout, "exact")) {
        double dx = resolveDoubleDispStrArg(ds.getTagArg("p", 3), submod, 0);
        double dy = resolveDoubleDispStrArg(ds.getTagArg("p", 4), submod, 0);
        x += dx;
        y += dy;
    }
    else if (!strcmp(layout, "r") || !strcmp(layout, "row")) {
        // perhaps we should use the size of the 1st element in the vector?
        double dx = resolveDoubleDispStrArg(ds.getTagArg("p", 3), submod, 2*sx);
        x += submod->getIndex()*dx;
    }
    else if (!strcmp(layout, "c") || !strcmp(layout, "col") || !strcmp(layout, "column")) {
        double dy = resolveDoubleDispStrArg(ds.getTagArg("p", 3), submod, 2*sy);
        y += submod->getIndex()*dy;
    }
    else if (!strcmp(layout, "m") || !strcmp(layout, "matrix")) {
        // perhaps we should use the size of the 1st element in the vector?
        int columns = resolveLongDispStrArg(ds.getTagArg("p", 3), submod, 5);
        double dx = resolveDoubleDispStrArg(ds.getTagArg("p", 4), submod, 2*sx);
        double dy = resolveDoubleDispStrArg(ds.getTagArg("p", 5), submod, 2*sy);
        if (columns < 1)
            columns = 1;
        x += (submod->getIndex() % columns)*dx;
        y += (submod->getIndex() / columns)*dy;
    }
    else if (!strcmp(layout, "i") || !strcmp(layout, "ri") || !strcmp(layout, "ring")) {
        // perhaps we should use the size of the 1st element in the vector?
        int vectorSize = submod->getVectorSize();
        double rx = resolveDoubleDispStrArg(ds.getTagArg("p", 3), submod, (sx+sy)*vectorSize/4);
        double ry = resolveDoubleDispStrArg(ds.getTagArg("p", 4), submod, rx);

        x += rx - rx*sin(submod->getIndex()*2*PI/vectorSize);
        y += ry - ry*cos(submod->getIndex()*2*PI/vectorSize);
    }
    else {
        throw cRuntimeError("发现了'p'关键字配置错误 '%s'，显示字符串格式如下 \"%s\"", layout, ds.str());
    }

    x *= zoomFactor;
    y *= zoomFactor;
}

void ModuleLayouter::clearLayout(cModule *module)
{
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it)
        modulePositions.erase(*it);
}

void ModuleLayouter::forgetPosition(cModule *submodule)
{
    modulePositions.erase(submodule);
}

void ModuleLayouter::refreshPositionFromDS(cModule *submodule)
{
    bool explicitCoords, obeysLayout;
    double x, y, sx, sy;

    getSubmoduleCoords(submodule, explicitCoords, obeysLayout, x, y, sx, sy);

    if (explicitCoords)
        modulePositions[submodule] = QPointF(x, y);
}

void ModuleLayouter::incrementLayoutSeed(cModule *module)
{
    std::string fullName = getObjectFullTypeName(module);

    if (layoutSeeds.find(fullName) == layoutSeeds.end())
        layoutSeeds[fullName] = 1; // must not use 0, the LCGRandom PRNG in the advanced layouter can't take that!

    ++layoutSeeds[fullName];
}

// 在ModuleCanvasViewer的doSetObject（）函数中，设置对象之后，确定每个模块都是已经设置好位置了；
void ModuleLayouter::ensureLayouted(cModule *module)
{
    if (!module)
        return;

    bool needsLayout = false;

    // 遍历每个子模块，如果那个模块在映射表中没有，则需要继续
    // loop through all submodules, and check if there are any that we don't have a valid position for
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it)
        if (modulePositions.find(*it) == modulePositions.end()) {
            needsLayout = true;
            break;
        }

    // if not, we have nothing to do
    // 检查完，如果都在映射表中有了，那就是每个都能确定位置了；
    if (!needsLayout)
        return;

    // recalculate layout, using coordinates in submodPosMap as "fixed" nodes --
    // only new nodes are re-layouted

    // Note trick avoid calling getDisplayString() directly because it'd cause
    // the display string object inside cModule to spring into existence
    const cDisplayString blank;
    const cDisplayString& ds = module->hasDisplayString() && module->parametersFinalized() ? module->getDisplayString() : blank;

    // create and configure layouter object
    LayouterChoice choice = getQtenv()->opt->layouterChoice;
    if (choice == LAYOUTER_AUTO) {
        const int LIMIT = 20;  // note: on test/anim/dynamic2, Advanced is already very slow with 30-40 modules
        int submodCountLimited = 0;
        for (cModule::SubmoduleIterator it(module); !it.end() && submodCountLimited < LIMIT; ++it)
            submodCountLimited++;
        choice = submodCountLimited >= LIMIT ? LAYOUTER_FAST : LAYOUTER_ADVANCED;
    }

    // 这里决定使用哪一个布局管理器来自动布局
    GraphLayouter *graphLayouter = choice == LAYOUTER_FAST ?
                (GraphLayouter *)new BasicSpringEmbedderLayout() :
                (GraphLayouter *)new ForceDirectedGraphLayouter();


    std::string fullName = getObjectFullTypeName(module);

    if (layoutSeeds.find(fullName) == layoutSeeds.end())
        layoutSeeds[fullName] = 1; // must not use 0, the LCGRandom PRNG in the advanced layouter can't take that!

    graphLayouter->setSeed(layoutSeeds[fullName]);

    // background size
    double sx = resolveDoubleDispStrArg(ds.getTagArg("bgb", 0), module, 0);
    double sy = resolveDoubleDispStrArg(ds.getTagArg("bgb", 1), module, 0);
    double border = 30;
    if (sx != 0 && sx < 2*border)
        border = sx/2;
    if (sy != 0 && sy < 2*border)
        border = sy/2;
    graphLayouter->setSize(sx, sy, border);
    // TODO support "bgp" tag ("background position")

    // loop through all submodules, get their sizes and positions and feed them into layouting engine
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it) {
        cModule *submod = *it;

        bool explicitCoords, obeysLayout;
        double x, y, sx, sy;

        // 一般都是啥也不写，则默认使用默认的图标，标准大小，并且返回的位置是（0,0）
        getSubmoduleCoords(submod, explicitCoords, obeysLayout, x, y, sx, sy);

        // add node into layouter:
        if (explicitCoords) {
            // e.g. "p=120,70" or "p=140,30,ring"
            graphLayouter->addFixedNode(submod->getId(), x, y, sx, sy);
        }
        // 这行很少被执行吧？
        else if (modulePositions.find(submod) != modulePositions.end()) {
            // reuse coordinates from previous layout
            QPointF pos = modulePositions[submod];
            graphLayouter->addFixedNode(submod->getId(), pos.x(), pos.y(), sx, sy);
        }
        else if (obeysLayout) {
            // all modules are anchored to the anchor point with the vector's name
            // e.g. "p=,,ring"
            graphLayouter->addAnchoredNode(submod->getId(), submod->getName(), x, y, sx, sy);
        }
        else {

            // 这行才是最经常执行的？
            graphLayouter->addMovableNode(submod->getId(), sx, sy);
        }
    }

    // 上述一段执行完之后，每个模块都有了一个至少的默认大小，但是位置还是不能确定的，基本都是(0, 0)
    // 下面一段是将各个模块之间的GATE链接都加一个EDGE

    // add connections into the layouter, too
    bool atParent = false;
    for (cModule::SubmoduleIterator it(module); !atParent; ++it) 
    {
        cModule *mod = !it.end() ? *it : (atParent = true, module);

        for (cModule::GateIterator git(mod); !git.end(); ++git) 
        {
            cGate *gate = *git;
            cGate *destGate = gate->getNextGate();
            if (gate->getType() == (atParent ? cGate::INPUT : cGate::OUTPUT) && destGate) 
            {
                cModule *destMod = destGate->getOwnerModule();
                if (mod == module && destMod == module)  // 同一个模块中的传输，啥也不需要做，也就是自发自收，不同的门
                {
                    // nop
                }
                else if (destMod == module) 
                { 
                     // 从某一个模块，发送到当前模块了；
                    graphLayouter->addEdgeToBorder(mod->getId());
                }
                else if (destMod->getParentModule() != module) {
                    // connection goes to a module under a different parent!
                    // this in fact violates module encapsulation, but let's
                    // accept it nevertheless. Just skip this connection.
                }
                else if (mod == module) 
                {
                    // 从当前模块发送到别的模块
                    graphLayouter->addEdgeToBorder(destMod->getId());
                }
                else 
                {
                    graphLayouter->addEdge(mod->getId(), destMod->getId());
                }
            }
        }
    }

    // checking if any of the submodules already have a position
    // 这里先看看，如果位置映射表中，有节点了，则设置为FALSE
    // 一般执行到这里，都是FASLE
    bool fullLayout = true;
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it)
        if (modulePositions.find(*it) != modulePositions.end())  // 如果找到了，则执行下一句
            fullLayout = false;

    // 这段代码是如果已经位置确定完毕，并且需要演示布局过程的话
    if (fullLayout && getQtenv()->opt->showLayouting) {
        // set up layouter environment (responsible for "Stop" button handling and visualizing the layouting process)
        QGraphicsScene *layoutingScene = new QGraphicsScene;

        { // block is to force destruction of the layouting enironment
            QtenvGraphLayouterEnvironment qtenvEnvironment(module, ds, layoutingScene);
            connect(this, SIGNAL(stop()), &qtenvEnvironment, SLOT(stop()));

            emit layoutVisualisationStarts(module, layoutingScene);

            graphLayouter->setEnvironment(&qtenvEnvironment);

            graphLayouter->execute();

            emit layoutVisualisationEnds(module);
        }

        delete layoutingScene;
    }
    else {
        // we still have to set something for the layouter if visualisation is disabled.
        InteractiveTimeoutBasicGraphLayouterEnvironment basicEnvironment;
        basicEnvironment.setTimeout(5);

        graphLayouter->setEnvironment(&basicEnvironment);

        graphLayouter->execute();
    }

    bool changed = false;
    // fill the map with the results
    for (cModule::SubmoduleIterator it(module); !it.end(); ++it) {
        cModule *submod = *it;

        QPointF pos;
        double x, y;
        graphLayouter->getNodePosition(submod->getId(), x, y);
        pos.setX(x);
        pos.setY(y);

        // 新增加，或者位置变更了，设置更改标记
        if (modulePositions.find(submod) == modulePositions.end() || modulePositions[submod] != pos)
            changed = true;

        modulePositions[submod] = pos;
    }

    // XXX is this needed?
    //layoutSeeds[fullName] = graphLayouter->getSeed();

    delete graphLayouter;

    if (changed)
        emit moduleLayoutChanged(module);
}

// drawsubmodules，绘制子模块时候，需要从映射中提取位置信息；
QPointF ModuleLayouter::getModulePosition(cModule *module, double zoomFactor)
{
    auto it = modulePositions.find(module);
    return it == modulePositions.end() ? QPointF(NAN, NAN) : (it->second * zoomFactor);
}

QRectF ModuleLayouter::getModuleRectangle(cModule *module, double zoomFactor, double imageSizeFactor)
{
    bool xplicit, obeys;
    double x, y, sx, sy;
    getSubmoduleCoords(module, xplicit, obeys, x, y, sx, sy, zoomFactor, imageSizeFactor);
    // using only the size from the above, position from our layouted map

    QPointF center = getModulePosition(module, zoomFactor);

    return QRectF(center.x() - sx/2, center.y() - sy/2, sx, sy);
}

void ModuleLayouter::fullRelayout(cModule *module)
{
    clearLayout(module);
    incrementLayoutSeed(module);
    ensureLayouted(module);
}

void ModuleLayouter::loadSeeds()
{
    auto env = getQtenv();
    auto seeds = env->getKeysInPrefGroup("LayoutSeeds");

    for (auto s : seeds)
        layoutSeeds[s.toStdString()] = env->getPref("LayoutSeeds/" + s, 1).toInt();

    /* // DEBUG OUTPUT
    qDebug() << "seeds after load:";
    for (auto p : layoutSeeds)
        qDebug() << p.first.c_str() << " -> " << p.second;
    qDebug() << "----";
    */
}

void ModuleLayouter::saveSeeds()
{
    /* // DEBUG OUTPUT
    qDebug() << "seeds before save:";
    for (auto p : layoutSeeds)
        qDebug() << p.first.c_str() << " -> " << p.second;
    qDebug() << "----";
    */

    auto env = getQtenv();

    for (auto p : layoutSeeds)
        env->setPref(QString("LayoutSeeds/") + p.first.c_str(), p.second);
}

} // namespace qtenv
} // namespace omnetpp
