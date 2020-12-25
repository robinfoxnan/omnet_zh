//==========================================================================
//  TREEITEMMODEL.CC - part of
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

#include "treeitemmodel.h"
#include "envir/visitor.h"
#include "omnetpp/cexception.h"
#include "omnetpp/cmodule.h"
#include "omnetpp/cmessage.h"
#include "omnetpp/csimplemodule.h"
#include "omnetpp/cwatch.h"
#include "omnetpp/cqueue.h"
#include "omnetpp/cchannel.h"
#include "omnetpp/coutvector.h"
#include "omnetpp/cstatistic.h"
#include "omnetpp/ccanvas.h"
#include "omnetpp/cfutureeventset.h"
#include "inspectorfactory.h"
#include "qtenv.h"
#include "qtutil.h"
#include "inspectorutil.h"
#include "mainwindow.h"
#include <QIcon>
#include <QMainWindow>

#include <QDebug>

namespace omnetpp {
namespace qtenv {

TreeItemModel::TreeItemModel(QObject *parent) :
    QAbstractItemModel(parent),
    rootNode(nullptr)
{
}

void TreeItemModel::setRootObject(cObject *root)
{
    beginResetModel();
    if (root == nullptr) {
    }  // qDebug() << "TreeItemModel::setRootObject: root is null";}//TODO log: TreeItemModel::setRootObject: root is null
    rootNode = root;
    endResetModel();
}

// 这个函数用来生成一个QModelIndex，将父节点的对象，以及行号封装到一起
// 这里需要理解的一点是：数据节点指针是关联到index内部的，和MFC类似
QModelIndex TreeItemModel::index(int row, int, const QModelIndex& parent) const
{
    // 无根节点返回一个空值，
    // qDebug() << "index" << row << 1;
    if (!rootNode)
        return QModelIndex();

    // 如果有根节点, 不提供索引，则返回根节点本身；
    // 如果有所以，则根据父节点计算子节点指针
    cObject *node = getObjectFromIndex(parent);

    // 使用基类函数创建一个索引，关联上一句查到的指针
    return createIndex(row, 0, node);
}

// 从索引返回一个对象，
// 无效节点返回根节点
// 有效，则将索引关联的数据指针拿到父节点，然后把子节点集合拿到，取出索引那个节点
cObject *TreeItemModel::getObjectFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return rootNode;

    cObject *parent = static_cast<cObject *>(index.internalPointer());
    cCollectChildrenVisitor visitor(parent);
    visitor.process(parent);
    cObject **objs = visitor.getArray();
    return visitor.getArraySize() > index.row() ? objs[index.row()] : nullptr;
}

QModelIndex TreeItemModel::parent(const QModelIndex& index) const
{
    // qDebug() << "parent";

    cObject *parent = index.isValid() ? static_cast<cObject *>(index.internalPointer()) :
        rootNode;
    if (!parent)
        return QModelIndex();
    cObject *grandParent = parent->getOwner();
    if (!grandParent)
        return QModelIndex();

    int row = findObjectInParent(parent, grandParent);
    if (row != -1)
        return createIndex(row, 0, grandParent);

    return QModelIndex();
}

int TreeItemModel::findObjectInParent(cObject *obj, cObject *parent) const
{
    cCollectChildrenVisitor visitor(parent);
    visitor.process(parent);
    cObject **objs = visitor.getArray();

    for (int i = 0; i < visitor.getArraySize(); ++i)
        if (objs[i] == obj)
            return i;


    return -1;
}

int TreeItemModel::rowCount(const QModelIndex& parent) const
{
    // qDebug() << "RowCount";
    cObject *node = getObjectFromIndex(parent);
    if (!node)
        return 0;

    cCountChildrenVisitor visitor(node);
    visitor.process(node);
    // qDebug() << visitor.getCount();
    return visitor.getCount();
}

// QVariant类型比较强大，可以保存各种类型，包括定制类以及指针，
// 这里是根据显示的不同类型，返回不同类型数据
QVariant TreeItemModel::data(const QModelIndex& index, int role) const
{
    // qDebug() << "data" << index;
    cObject *node = getObjectFromIndex(index);
    if (!node)
        return QVariant();

    switch (role) {

        // 显示节点名字时候返回：短名字（类名）id=....
        case Qt::DisplayRole: {
            long id = getObjectId(node);
            return node->getFullName() + QString(" (") + getObjectShortTypeName(node) + ")"
                    + (id >= 0 ? " id=" + QString::number(id) : "");
        }
        // 返回图标
        case Qt::DecorationRole:
            return getObjectIcon(node);

        // 请求用户数据
        case Qt::UserRole:
            // returning the raw cObject* - needed to preserve object tree node expansion state
            return QVariant::fromValue(node);

        // 请求提示信息
        case Qt::ToolTipRole: {
            auto info = node->str();
            return QString("(") + getObjectShortTypeName(node) + ") " + node->getFullName()
                    + (info.empty() ? "" : (", " + info).c_str());
        }
        default:
            return QVariant();
    }
}

bool TreeItemModel::hasChildren(const QModelIndex& parent) const
{
    cObject *parentNode = getObjectFromIndex(parent);
    if (!parentNode)
        return false;

    cHasChildrenVisitor visitor(parentNode);
    visitor.process(parentNode);
    return visitor.getResult();
}

// 核心的入口函数在这里
// list用作返回值列表
// idx作为深度递归的一个参数，第一级调用时候是一个无效的索引
// 初始化时候，根节点是要添加的其子节点如果没有展开，则只返回1条。
void TreeItemModel::getExpandedItems(QTreeView *view, QList<QVariant>& list, QModelIndex idx)
{
    if (!idx.isValid()) {
        // 根节点始终是要添加的，
        list.append(idx.data(Qt::UserRole));
        // this is the root node, so we index the model itself in the iteration
        // 这里返回的是根节点的下属行数，这里其实用rowcount和list两次迭代访问，时间效率比较低
        for (int row = 0; row < rowCount(); ++row)
            getExpandedItems(view, list, index(row, 0));
    }
    else
        if (view->isExpanded(idx)) {
            // adding the pointer of the current node to the list because it is expanded in the view
            list.append(idx.data(Qt::UserRole));

            // this is not the root node, so we iterate on all of its children
            for (int row = 0; row < rowCount(idx); ++row)
                getExpandedItems(view, list, idx.child(row, 0));
        }
}

void TreeItemModel::expandItems(QTreeView *view, const QList<QVariant>& list, QModelIndex idx)
{
    if (list.contains(idx.data(Qt::UserRole))) {
        // expanding the node in the view because it is in the list
        view->expand(idx);

        if (idx.isValid()) {
            // this is not the root node, so we iterate on all of its children
            for (int row = 0; row < rowCount(idx); ++row) {
                expandItems(view, list, idx.child(row, 0));
            }
        }
        else {
            // this is the root node, so we index the model itself in the iteration
            for (int row = 0; row < rowCount(); ++row) {
                expandItems(view, list, index(row, 0));
            }
        }
    }
}

}  // namespace qtenv
}  // namespace omnetpp

