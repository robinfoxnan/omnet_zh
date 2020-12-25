//==========================================================================
//  IOSGVIEWER.CC - part of
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

#include <iostream>
#include "iosgviewer.h"
#include "envir/fsutils.h"
#include "common/ver.h"

namespace omnetpp {
namespace qtenv {

// the osg implementation library will set this pointer to a real factory in it
QTENV_API IOsgViewerFactory *osgViewerFactory = nullptr;

class DummyOsgViewerFactory: public IOsgViewerFactory
{
public:
    virtual IOsgViewer *createViewer() { return new DummyOsgViewer(); }
    virtual void shutdown() {}

    virtual void refNode(osg::Node *) {}
    virtual void unrefNode(osg::Node *) {}
};

DummyOsgViewerFactory dummyOsgFactory;

void IOsgViewer::ensureViewerFactory()
{
    if (!osgViewerFactory) {
        osgViewerFactory = &dummyOsgFactory;

        std::cout << "加载OSG界面库... " << std::endl;

        try {
            loadExtensionLibrary("oppqtenv-osg");
        }
        catch (cRuntimeError &e) {
            std::cout << "失败: " << e.what() << std::endl;
        }

        std::cout << std::endl;

        /*
        // Not throwing an exception here, because that will take the simulation into an "ERROR"
        // state, and will not let it continue - even though it could - , but after rebuilding the
        // network, it will.
        // And not showing a dialog at all because then it could get annoying when the user
        // doesn't have/want/care about 3D at all, but the model (INET with the 3D visualizations
        // for example) creates a cOsgCanvas - which is possible now. And it won't be a dummy
        // implementation anymore. And the dummy viewer will show an error anyways.
        if (osgViewerFactory == &dummyOsgFactory)
            getQtenv()->confirm(Qtenv::WARNING,
                                "Could not load the OSG viewer library (qtenv-osg).\n"
                                "Continuing without the ability to view the scenes of cOsgCanvas objects.\n"
                                "(Is " OMNETPP_PRODUCT " compiled with OSG support enabled?)");
        */
    }
}

IOsgViewer *IOsgViewer::createOne()
{
    ensureViewerFactory();

    auto viewer = osgViewerFactory->createViewer();

    if (!viewer)
        throw cRuntimeError("无法创建OsgViewer.");

    return viewer;
}

void IOsgViewer::uninit()
{
    // if no viewers were created (so the impl. lib. is not loaded), then no-op
    if (osgViewerFactory)
        osgViewerFactory->shutdown();
}

void IOsgViewer::refNode(osg::Node *node)
{
    ensureViewerFactory();

    osgViewerFactory->refNode(node);
}

void IOsgViewer::unrefNode(osg::Node *node)
{
    ensureViewerFactory();

    osgViewerFactory->unrefNode(node);
}

bool IOsgViewer::isOsgPreferred()
{
    return osgViewerFactory != nullptr && osgViewerFactory != &dummyOsgFactory;
}

void DummyOsgViewer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    auto rect = QRect({0,0}, size());

    QPalette pal = palette();
    pal.setCurrentColorGroup(QPalette::Disabled);

    painter.fillRect(rect, pal.window());

    painter.setPen(QPen(pal.text().color()));
    painter.drawText(rect.adjusted(50, 50, -50, -50), // so the toolbar won't ob
                     "OSG 界面库无法加载\n"
                     "( " OMNETPP_PRODUCT " 编译时候开启OSG选项了么？)");
}

} // qtenv
} // omnetpp
