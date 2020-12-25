//==========================================================================
//  INSPECTORUTIL.CC - part of
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

#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <common/stringutil.h>
#include <omnetpp/cobject.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/cmodule.h>
#include <omnetpp/cmessage.h>
#include "qtenv.h"
#include "inspector.h"
#include "inspectorfactory.h"
#include "inspectorutil.h"
#include "preferencesdialog.h"
#include "genericobjectinspector.h"
#include "mainwindow.h"

namespace omnetpp {
using namespace common;
namespace qtenv {

QVector<InspectorType> InspectorUtil::supportedInspTypes(cObject *object)
{
    using namespace qtenv;
    QVector<InspectorType> insp_types;
    cRegistrationList *a = inspectorfactories.getInstance();
    for (int i = 0; i < a->size(); i++) {
        InspectorFactory *ifc = static_cast<InspectorFactory *>(a->get(i));
        if (ifc->supportsObject(object)) {
            int k;
            for (k = 0; k < insp_types.size(); k++)
                if (insp_types[k] == ifc->getInspectorType())
                    break;

            if (k == insp_types.size())  // not yet in vector, insert
                insp_types.push_front(ifc->getInspectorType());
        }
    }

    return insp_types;
}

Inspector *InspectorUtil::getContainingInspector(QWidget *widget)
{
    QWidget *curr = widget;
    while (curr) {
        Inspector *insp = dynamic_cast<Inspector *>(curr);
        if (insp)
            return insp;
        curr = curr->parentWidget();
    }
    return nullptr;
}

void InspectorUtil::fillInspectorContextMenu(QMenu *menu, cObject *object, Inspector *insp)
{
    // add "Go Into" and "View in Embedded" if applicable
    QString name = object->getFullName();
    if (insp && object != insp->getObject()) {
        if (insp->supportsObject(object)) {
            menu->addAction("转到 '" + name + "'", insp, SLOT(goUpInto()))
                ->setData(QVariant::fromValue(object));
        }

        if (insp != getQtenv()->getMainObjectInspector())
            menu->addAction("查看 '" + name + "' 经由对象查看器", getQtenv()->getMainObjectInspector(), SLOT(goUpInto()))
                ->setData(QVariant::fromValue(object));

        menu->addSeparator();
    }

    // add inspector types supported by the object
    for (InspectorType type : supportedInspTypes(object)) {
        QString label = getInspectMenuLabel(type);
        if (label.isEmpty())
            qDebug() << "Unsupported inspector type " << type << " in context menu";
        label += QString("，对象： '") + name + "'";
        QAction *action = menu->addAction(label, getQtenv(), SLOT(inspect()));
        action->setData(QVariant::fromValue(InspectActionData{object, type}));
    }

    // add "run until" menu items
    if (dynamic_cast<cSimpleModule *>(object) || dynamic_cast<cModule *>(object)) {
        menu->addSeparator();
        QAction *action = menu->addAction(QString("运行到模块下一个事件，模块： '") + name + "'", getQtenv(), SLOT(runUntilModule()));
        action->setData(QVariant::fromValue(RunUntilNextEventActionData{object, RUNMODE_NORMAL, insp}));
        action = menu->addAction(QString("快速运行到模块下一个事件，模块： '") + name + "'", getQtenv(), SLOT(runUntilModule()));
        action->setData(QVariant::fromValue(RunUntilNextEventActionData{object, RUNMODE_FAST, insp}));
    }

    cMessage *msg = dynamic_cast<cMessage *>(object);
    if (msg) {
        QAction *action;
        if (msg->isScheduled()) {
            menu->addSeparator();
            action = menu->addAction(QString("运行到提交消息，模块： '") + name + "'", getQtenv(), SLOT(runUntilMessage()));
            action->setData(QVariant::fromValue(RunUntilActionData{object, RUNMODE_NORMAL}));
            action = menu->addAction(QString("快速运行到提交消息，模块： '") + name + "'", getQtenv(), SLOT(runUntilMessage()));
            action->setData(QVariant::fromValue(RunUntilActionData{object, RUNMODE_FAST}));
            action = menu->addAction(QString("极速运行到提交消息，模块： '") + name + "'", getQtenv(), SLOT(runUntilMessage()));
            action->setData(QVariant::fromValue(RunUntilActionData{object, RUNMODE_EXPRESS}));
        }
        menu->addSeparator();
        action = menu->addAction(QString("从动画中排除消息，模块： '") + name + "' ", getQtenv(), SLOT(excludeMessage()));
        action->setData(QVariant::fromValue(object));
    }

    cComponent *comp = dynamic_cast<cComponent *>(object);
    if (comp) {
        menu->addSeparator();
        QMenu *subMenu = menu->addMenu(QString("设置日志级别，模块： '") + name + "' 和子模块...");
        QActionGroup *logLevelActionGroup = new QActionGroup(menu);

        addLoglevel(LOGLEVEL_TRACE, "Trace", comp, logLevelActionGroup, subMenu);
        addLoglevel(LOGLEVEL_DEBUG, "Debug", comp, logLevelActionGroup, subMenu);
        addLoglevel(LOGLEVEL_DETAIL, "Detail", comp, logLevelActionGroup, subMenu);
        addLoglevel(LOGLEVEL_INFO, "Info", comp, logLevelActionGroup, subMenu);
        addLoglevel(LOGLEVEL_WARN, "Warn", comp, logLevelActionGroup, subMenu);
        addLoglevel(LOGLEVEL_ERROR, "Error", comp, logLevelActionGroup, subMenu);
        addLoglevel(LOGLEVEL_FATAL, "Fatal", comp, logLevelActionGroup, subMenu);
        subMenu->addSeparator();
        addLoglevel(LOGLEVEL_OFF, "Off", comp, logLevelActionGroup, subMenu);
    }

    // add utilities menu
    menu->addSeparator();
    QMenu *subMenu = menu->addMenu(QString("相关操作，对象： '") + name + "'");
    QAction *action = subMenu->addAction("拷贝指针并转换 (调试器使用)", getQtenv(), SLOT(utilitiesSubMenu()));
    action->setData(QVariant::fromValue(CopyActionData{object, COPY_PTRWITHCAST}));
    action = subMenu->addAction("拷贝指针的值 (调试器使用)", getQtenv(), SLOT(utilitiesSubMenu()));
    action->setData(QVariant::fromValue(CopyActionData{object, COPY_PTR}));
    subMenu->addSeparator();

    action = subMenu->addAction("拷贝全路径", getQtenv(), SLOT(utilitiesSubMenu()));
    action->setData(QVariant::fromValue(CopyActionData{object, COPY_FULLPATH}));
    action = subMenu->addAction("拷贝名字", getQtenv(), SLOT(utilitiesSubMenu()));
    action->setData(QVariant::fromValue(CopyActionData{object, COPY_FULLNAME}));
    action = subMenu->addAction("拷贝类名", getQtenv(), SLOT(utilitiesSubMenu()));
    action->setData(QVariant::fromValue(CopyActionData{object, COPY_CLASSNAME}));

    menu->addSeparator();
    menu->addAction("查找对象，模块： '" + name + "'", getQtenv()->getMainWindow(), SLOT(on_actionFindInspectObjects_triggered()))
            ->setData(QVariant::fromValue(object));
}

void InspectorUtil::addLoglevel(LogLevel level, QString levelInStr, cComponent *comp, QActionGroup *logLevelActionGroup,
                                QMenu *subMenu)
{
    QAction *action = subMenu->addAction(levelInStr, getQtenv(), SLOT(setComponentLogLevel()));
    action->setData(QVariant::fromValue(ComponentLogActionData{comp, level}));
    action->setCheckable(true);
    action->setChecked(comp->getLogLevel() == level);
    action->setActionGroup(logLevelActionGroup);
}

QMenu *InspectorUtil::createInspectorContextMenu(cObject *object, Inspector *insp)
{
    QVector<cObject *> obj;
    obj.push_back(object);
    return createInspectorContextMenu(obj, insp);
}

// 在树列表中使用这个函数来创建右键菜单，主要可能是涉及的对象太复杂了；
QMenu *InspectorUtil::createInspectorContextMenu(QVector<cObject *> objects, Inspector *insp)
{
    QMenu *menu = new QMenu();
    QFont font = getQtenv()->getBoldFont();
    menu->setStyleSheet("font: " + QString::number(font.pointSize()) + "pt \"" + font.family() + "\"");

    // If there are more than one ptrs, remove the inspector object's own ptr:
    // when someone right-clicks a submodule icon, we don't want the compound
    // module to be in the list.
    if (insp && objects.size() > 1) {
        cObject *object = insp->getObject();
        if (objects.indexOf(object) >= 0 && objects.indexOf(object) < objects.size())
            objects.remove(objects.indexOf(object));
    }

    if (objects.size() == 1) {
        fillInspectorContextMenu(menu, objects[0], insp);
    }
    else if (objects.size() > 1) {
        const int maxObjects = 20;

        // then create a submenu for each object
        for (int i = 0; i < std::min(maxObjects, objects.size()); ++i) {
            cObject *object = objects[i];

            const char *name = object->getFullName();
            const QString &shortTypeName = getObjectShortTypeName(object);
            QString infoStr = shortTypeName + QString(", ") + object->str().c_str();
            if (infoStr.size() > 30) {
                infoStr.truncate(30);
                infoStr += "...";
            }

            std::string baseClass = getObjectBaseClass(object);
            QString label;
            if (baseClass == "cGate") {
                cGate *nextGate = static_cast<cGate *>(object)->getNextGate();
                const char *nextGateName = nextGate->getFullName();
                cObject *owner = object->getOwner();
                const char *ownerName = owner->getFullName();
                cObject *nextGateOwner = nextGate->getOwner();
                const char *nextGateOwnerName = nextGateOwner->getFullName();

                label = ownerName + QString(".") + name + " --> " + nextGateOwnerName + "." + nextGateName;
            }
            else if (baseClass == "cMessage") {
                QString shortInfo = getMessageShortInfoString(static_cast<cMessage *>(object));
                label = label + name + " (" + shortTypeName + ", " + shortInfo + ")";
            }
            else
                label = label + name + " (" + infoStr + ")";

            QMenu *subMenu = menu->addMenu(label);
            fillInspectorContextMenu(subMenu, object, insp);
        }
        if (objects.size() > maxObjects) {
            menu->addAction("对象太多, 仅显示 "
                            + QString::number(maxObjects) + "个，共"
                            + QString::number(objects.size()))->setEnabled(false);
        }
    }

    if (insp) {
        cObject *object = insp->getObject();
        if (object) {
            cObject *parent = dynamic_cast<cComponent *>(object) ? ((cComponent *)object)->getParentModule() : object->getOwner();
            if (parent && insp->supportsObject(parent)) {
                menu->addSeparator();
                QAction *action = menu->addAction("向上", insp, SLOT(goUpInto()));
                action->setData(QVariant::fromValue(parent));
            }
        }
    }

    return menu;
}

void InspectorUtil::copyToClipboard(cObject *object, eCopy what)
{
    switch (what) {
        case COPY_PTR: {
            void *address = static_cast<void *>(object);
            std::stringstream ss;
            ss << address;
            setClipboard(QString(ss.str().c_str()));
            break;
        }

        case COPY_PTRWITHCAST: {
            void *address = static_cast<void *>(object);
            std::stringstream ss;
            ss << address;
            setClipboard(QString("((") + object->getClassName() + " *)" + ss.str().c_str() + ")");
            break;
        }

        case COPY_FULLPATH:
            setClipboard(object->getFullPath().c_str());
            break;

        case COPY_FULLNAME:
            setClipboard(object->getFullName());
            break;

        case COPY_CLASSNAME:
            setClipboard(object->getClassName());
            break;
    }
}

void InspectorUtil::setClipboard(QString str)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(str);
}

void InspectorUtil::preferencesDialog(eTab defaultPage)
{
    PreferencesDialog *prefDialog = new PreferencesDialog(defaultPage, getQtenv()->getMainWindow());
    prefDialog->exec();
    delete prefDialog;
}

QString InspectorUtil::getInspectMenuLabel(InspectorType type)
{
    switch(type) {
        case INSP_DEFAULT: return "打开合适的视图";
        case INSP_OBJECT: return "打开详情";
        case INSP_GRAPHICAL: return "打开图像视图";
        case INSP_LOG: return "打开组件日志";
        case INSP_OBJECTTREE: return "打开对象树";
        default: return "";
    }
}

QString InspectorUtil::formatDouble(double num)
{
    // Don't use 'g' format because it ignores precision for the scientific notation.
    // The other two formats don't omit trailing zeroes.
    return QString::asprintf("%.16g", num);
}

}  // namespace qtenv
}  // namespace omnetpp

