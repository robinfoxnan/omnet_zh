//==========================================================================
//  graphicsitems.h - part of
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

#ifndef __OMNETPP_QTENV_GRAPHICSITEMS_H
#define __OMNETPP_QTENV_GRAPHICSITEMS_H

#include <QGraphicsObject>
#include <QGraphicsEffect>
#include <QFont>
#include <QPen>
#include <QTimer>
#include "qtenvdefs.h"

namespace omnetpp {
namespace qtenv {

// these are the custom data "slot" indices used in QGraphicsItems
constexpr int ITEMDATA_COBJECT = 1;
// see modulecanvasviewer.cc for why this is necessary, and setToolTip isn't usable
constexpr int ITEMDATA_TOOLTIP = 2; // not used for figures anymore!

class QTENV_API ArrowheadItem : public QGraphicsPolygonItem
{
    double arrowWidth = 6;
    // This will offset the tail sideways. -1 is fully left, 1 is fully right.
    // 这将使尾部侧向偏移。-1表示完全左，1表示完全右
    double arrowSkew = 0;   // 倾斜
    double arrowLength = 4;
    double fillRatio = 0.75;

    void updatePolygon();

public:
    ArrowheadItem(QGraphicsItem *parent);

    QPainterPath shape() const override;

    // Sets the size of the arrow so it fits
    // a line of penWidth width well.
    void setSizeForPenWidth(double penWidth, double zoom = 1.0, double addSize = 8.0);

    void setEndPoints(const QPointF &start, const QPointF &end, double addAngle = 0);

    void setArrowWidth(double width);
    void setArrowLength(double length);
    void setArrowSkew(double skew);
    void setFillRatio(double ratio);
    void setColor(const QColor &color);
    void setLineWidth(double width);
};

// used in the ModuleInspector and some related classes
// 也仅仅是添加删除，绘图
// CanvasViewer类中，这个成员很重要啊
class QTENV_API GraphicsLayer : public QGraphicsObject {
    Q_OBJECT

public:
    QRectF boundingRect() const override { return QRectF(); /* doesn't matter */ }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override { /* nothing here... */ }
    void addItem(QGraphicsItem *item) { item->setParentItem(this); }
    void removeItem(QGraphicsItem *item) { item->setParentItem(nullptr); }
    void clear();
};


// Used in the ModuleInspector to make text more readable on a cluttered background,
// as well as on the canvas to implement the halo option for label and text figures.
// If the functionality is not enough, just implement more of QGraphicsSimpleTextItems
// functions, and forward them to one or both members accordingly.
// The occasional small "offset" of the outline relative to the text
// itself (mostly with small fonts) is likely caused by font hinting.
class QTENV_API OutlinedTextItem : public QGraphicsItem {
protected:
    // these are NOT PART of the scene, not even children of this object
    // we just misuse them in the paint method
    QGraphicsSimpleTextItem *outlineItem; // never has a Brush
    QGraphicsSimpleTextItem *fillItem; // never has a Pen

    QBrush bgBrush;
    bool haloEnabled = true;

public:
    OutlinedTextItem(QGraphicsItem *parent = nullptr);
    virtual ~OutlinedTextItem();

    QRectF boundingRect() const;
    QRectF textRect() const; // this would be the bounding box without the outline

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QFont font() const { return fillItem->font(); }
    void setFont(const QFont &font);
    QString text() const { return fillItem->text(); }
    void setText(const QString &text);
    QPen pen() const { return outlineItem->pen(); }
    void setPen(const QPen &pen);
    QBrush brush() const { return fillItem->brush(); }
    void setBrush(const QBrush &brush);
    QBrush backgroundBush() const { return bgBrush; }
    void setBackgroundBrush(const QBrush &brush);
    bool isHaloEnabled() const { return haloEnabled; }
    void setHaloEnabled(bool enabled);
};

class QTENV_API MultiLineOutlinedTextItem : public QGraphicsItem {
protected:

    Qt::Alignment alignment = Qt::AlignLeft; // only Left, Center and Right are accepted.

    // This needs to be a vector only so we can position the lines independently,
    // to left/middle/right align the text for the different positioning modes.
    // QGraphicsSimpleTextItem can't align its lines, while QGraphicsTextItem
    // makes it hard to draw the outline. So we do it manually, no big deal.
    // This must always contain at least one element, which is used to hold
    // the desired style info (font, colors, halo, etc.).
    std::vector<OutlinedTextItem *> textItems;

    void realignLines();

public:
    MultiLineOutlinedTextItem(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent) { setText(""); /* just to ensure at least one line item */ }

    QRectF boundingRect() const override { return childrenBoundingRect(); } // just to make positioning calculations easier
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override { /* nothing */ }

    void setText(const QString &text);
    void setAlignment(Qt::Alignment align);
    void setFont(const QFont &font);
    void setPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void setBackgroundBrush(const QBrush &brush);
    void setHaloEnabled(bool enabled);
};

// Label 在右下角设置一个缩放比例；
class QTENV_API ZoomLabel : public QGraphicsSimpleTextItem
{
private:
    double zoomFactor;

protected:
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) override;

public:
    ZoomLabel() { setText("缩放比例"); }  // ZoomLabel

    void setZoomFactor(double zoomFactor);
};

// XXX: Why not QGraphicsPathItem ?
// 老外接手的人也觉得很奇怪，为啥换了一个父类？
/*
通过继承QGraphicsItem类来定制自己的图元类，这样就可以根据自己的需求来
制作它的形状、颜色、属性以及交互等。
但是这样的图元会存在一个缺点，那就是不能使用Qt的信号/槽机制，
也不能使用Qt属性系统，因为QGraphicsItem并不继承于QObject.
缺少信号/槽这样好用的机制会使得自定义图元与其他对象交互变得很不方便，
不能使用属性系统也会让类失去更多的扩展性。总之不从QObject派生类会给类的使用带来很多的不利。
好在Qt已经考虑到了这些问题，QGraphicsObject应运而生。
QGraphicsObject同时继承于QObject和QGraphicsItem。
这样自定义图元既可以获得QGraphicsItem中写好的图元特性又能使用信号/槽机制方便信息传递。
*/
class QTENV_API BubbleItem : public QGraphicsObject {
    Q_OBJECT

protected:

    // the distance between the tip of the handle, and the bottom of the text bounding rectangle
    // includes the bottom margin (the yellow thingy will begin a margin size lower than this)
    static constexpr double vertOffset = 15;
    static constexpr double margin = 3; // also acts as corner rounding radius

    QString text;

    QPainterPath path;
    bool pathBuilt = false;

    void timerEvent(QTimerEvent *event) override { delete this;  /* BOOM! */ }

public:
    BubbleItem(QPointF position, const QString &text, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

}  // namespace qtenv
}  // namespace omnetpp


#endif // __OMNETPP_QTENV_GRAPHICSITEMS_H
