//==========================================================================
//  TIMELINEINSPECTOR.CC - part of
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

#include "timelineinspector.h"
#include <QHBoxLayout>
#include <QMenu>
#include "timelinegraphicsview.h"
#include "inspectorutil.h"
#include "inspectorfactory.h"
#include "qtenv.h"

#define emit

namespace omnetpp {
namespace qtenv {

// 这么小的类，为啥还要个工厂类？
class TimeLineInspectorFactory : public InspectorFactory
{
  public:
    TimeLineInspectorFactory(const char *name) : InspectorFactory(name) {}

    bool supportsObject(cObject *) override { return false; }
    InspectorType getInspectorType() override { return INSP_LOG; }
    double getQualityAsDefault(cObject *) override { return 0; }
    Inspector *createInspector(QWidget *parent, bool isTopLevel) override { return new TimeLineInspector(parent, isTopLevel, this); }
};

// 注册到工厂库
Register_InspectorFactory(TimeLineInspectorFactory);

// 构造函数就做了2件事：创建一个时间轴窗口，用布局grid展示，链接信号与槽。
TimeLineInspector::TimeLineInspector(QWidget *parent, bool isTopLevel, InspectorFactory *f) : Inspector(parent, isTopLevel, f)
{

    QGridLayout *layout = new QGridLayout(this);
    timeLine = new TimeLineGraphicsView();
    timeLine->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    timeLine->setScene(new QGraphicsScene());

    parent->setMinimumSize(20, 20);
    layout->addWidget(timeLine, 0, 0);
    layout->setMargin(0);

    connect(timeLine, SIGNAL(contextMenuRequested(QVector<cObject *>, QPoint)), this, SLOT(createContextMenu(QVector<cObject *>, QPoint)));
    connect(timeLine, SIGNAL(click(cObject *)), this, SLOT(setObjectToObjectInspector(cObject *))); // 单击事件，会在左侧显示
    connect(timeLine, SIGNAL(doubleClick(cObject *)), this, SLOT(openInspector(cObject *)));        // 双击，弹出新的窗口
    connect(getQtenv(), SIGNAL(fontChanged()), this, SLOT(onFontChanged()));
}

void TimeLineInspector::onFontChanged()
{
    timeLine->setTickLabelFont(getQtenv()->getTimelineFont());
    timeLine->setMessageLabelFont(getQtenv()->getTimelineFont());
}

void TimeLineInspector::createContextMenu(QVector<cObject *> objects, QPoint globalPos)
{
    if (objects.size()) {
        QMenu *menu = InspectorUtil::createInspectorContextMenu(objects, this);
        menu->exec(globalPos);
        delete menu;
    }
    else {
        QMenu *menu = new QMenu();
        menu->addAction("时间轴设置...", this, SLOT(runPreferencesDialog()));
        menu->exec(globalPos);
        delete menu;
    }
}

void TimeLineInspector::setObjectToObjectInspector(cObject *object)
{
    emit selectionChanged(object);
}

// 打开窗口，查看对象
void TimeLineInspector::openInspector(cObject *object)
{
    getQtenv()->inspect(object, INSP_OBJECT, true);
}

// 这个函数会重建场景，并绘制消息
void TimeLineInspector::refresh()
{
    timeLine->rebuildScene();
}

void TimeLineInspector::runPreferencesDialog()
{
    InspectorUtil::preferencesDialog(TAB_FILTERING);
}

double TimeLineInspector::getInitHeight()
{
    return timeLine->getInitHeight();
}

}  // namespace qtenv
}  // namespace omnetpp

