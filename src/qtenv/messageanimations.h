//==========================================================================
//  messageanimations.h - part of
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

#ifndef __OMNETPP_QTENV_MESSAGEANIMATIONS_H
#define __OMNETPP_QTENV_MESSAGEANIMATIONS_H

#include <vector>
#include <map>
#include <QString>
#include <QLineF>
#include "omnetpp/simtime_t.h"
#include "omnetpp/cgate.h"
#include "omnetpp/cmodule.h"
#include "qtenvdefs.h"

namespace omnetpp {

class cMessage;

namespace qtenv {

class ModuleInspector;
class MessageItem;
class ConnectionItem;
class Inspector;


struct QTENV_API PathEntry {
   cModule *from; // nullptr if descent
   cModule *to;   // nullptr if ascent
};
typedef std::vector<PathEntry> PathVec;


class QTENV_API Animation {
protected:
    // For holding animations.
    explicit Animation(double holdDuration): holding(true), holdDuration(holdDuration) { }

    // For non-holding, SimTime animations.
    explicit Animation(): holding(false) { }

    // If true, the animation will take place during a hold, and setProgress() will be called from update().
    // If false, during some amount of simtime without hold, and only update() should be used.
    // This is not a dynamic state variable, but a static property.
    // Not used for AnimationSequence, see isHolding in that class.
    // XXX consider moving this to MessageAnimation?
    const bool holding;

    // In animation time, ONLY AN ESTIMATE to show the user.
    // Real (internal) hold management is done with the requestHold()
    // and clearHold() methods of the AnimationManager.
    // This is nonzero only if holding is false.
    double holdDuration = 0;
    // In animation time, set in begin(). Not used if holding is false.
    double holdStartTime = 0;

    // a simple shortcut, forwards the request to the MessageAnimator
    void requestAnimationSpeed(double speed);

public:

    //                                       ,-------------------v
    // > PENDING -> init() -> WAITING -> begin() -> PLAYING -> end() -> FINISHED -> cleanup() -> DONE >
    //                                                 '|,      /'
    //                                                  update()
    enum State { PENDING, WAITING, PLAYING, FINISHED, DONE } state = PENDING;

    State getState() const { return state; } // see comment for state
    virtual bool isHolding() const { return holding; } // see comment for holding

    double getHoldPosition() const; // a [0..1] value, the progress in this animations hold
    // Basically getHoldPosition() >= 1.0, but that can't be used because of floating point inaccuracies.
    // This returning true doesn't mean that the hold request has been cancelled,
    // only that it should be called now, because the time has come for it.
    bool holdExpired() const;
    double getHoldEndTime() const; // see comment for holdDuration

    // All of these must be called from the subclasses' overrides!
    virtual void init(); // when the sequence is started
    virtual void begin(); // when this anim is started in the sequence
    virtual void update(); // called regularly while in PLAYING state
    virtual void end(); // when this animation is over (called from either begin() or update())
    virtual void cleanup(); // when the group is ended

    // Helper method to call the ones above in the right order.
    // Returns true if the animation has not ended yet.
    bool advance();

    // The parameter of the next 3 functions is not a ModuleInspector*,
    // so we can call them on all open inspectors easily, and they
    // perform an isModuleInspectorFor anyway.

    // The animations should make themselves visible in insp if applicable.
    // The inspector should be remembered and animated in until removed.
    // Also see MessageAnimator::addInspector.
    virtual void addToInspector(Inspector *insp) = 0;

    // This is different from a simple update(), animations should completely
    // rebuild the parts of them that are visible in insp, because it may
    // have been zoomed or relayouted, or the displayed module changed, etc...
    // Also see MessageAnimator::updateInspector.
    virtual void updateInInspector(Inspector *insp) = 0;

    // In this method the animations should completely clear any parts of them
    // that are visible in insp, so it can be safely closed, or the inspected
    // object can be changed.
    // Also see MessageAnimator::clearInspector.
    virtual void removeFromInspector(Inspector *insp) = 0;


    // True if currently no part of this animation is visible in any open inspectors.
    virtual bool isEmpty() const = 0;

    // True if this animation (tree) is currently, or will in the future,
    // show a representation of msg. Needed to avoid drawing the static
    // message item where it has not "arrived" yet in the animation.
    virtual bool willAnimate(cMessage *msg) = 0;

    // This is used to notify the animations about the creation of a
    // cMessage's private clone by the LogBuffer.
    virtual void messageDuplicated(cMessage *msg, cMessage *dup) = 0;

    // This is called when the message is delivered or deleted, so
    // the graphics items can remove their references to it.
    // If needed, animations might clone the message for later use.
    virtual void removeMessagePointer(cMessage *msg) = 0;

    // Only needed for debugging.
    virtual QString str() const = 0;

    virtual int getSourceModuleId() { return -1; }

    virtual ~Animation();
};

class QTENV_API AnimationSequence : public Animation
{
protected:
    std::vector<Animation *> parts;
    size_t currentPart = 0;

    // If this is set, the init() of the parts will be
    // called just before their begin() calls,
    // (at some time in this sequence's update())
    // and not in this sequence's init() or begin().
    // If not set, they will be called in the sequence's begin()
    // 如果推迟初始化，在各自分别初始化，如果不推迟，统一初始化
    bool deferPartsInit;  

    // Returns a new AnimationSequence which contains the parts of
    // this sequence, starting with index from, and removes those
    // parts from this sequence.
    // 从旧队列中间截断一个新的序列返回给调用者
    AnimationSequence *chopTail(size_t from);

public:
    // 显示构造函数，阻止隐式转换
    explicit AnimationSequence(bool deferPartsInit = false)
        : Animation(), deferPartsInit(deferPartsInit) { }

    // 在尾部添加一个动画，必要时进行初始化
    virtual void addAnimation(Animation *a);
    size_t getNumParts() const { return parts.size(); }

    // Removes the parts from the end that are holding,
    // and returns them in a new sequence.
    AnimationSequence *chopHoldingTail();

    // If this sequence is currently playing a part that is holding in nature.
    bool isHolding() const override;
    // If this sequence consists entierly of holding parts.
    bool isHoldingOnly() const;

    void begin() override;
    void update() override;
    void end() override;

    // These are applied to the parts.
    void addToInspector(Inspector *insp) override;
    void updateInInspector(Inspector *insp) override;
    void removeFromInspector(Inspector *insp) override;

    bool isEmpty() const override;
    bool willAnimate(cMessage *msg) override;
    void messageDuplicated(cMessage *msg, cMessage *dup) override;
    void removeMessagePointer(cMessage *msg) override;

    QString str() const override;

    int getSourceModuleId() override { return parts.empty() ? -1 : parts[0]->getSourceModuleId(); }

    ~AnimationSequence() override;
};

class QTENV_API MethodcallAnimation : public Animation
{
    // The caller and callee modules.
    cModule *srcMod, *destMod;
    // Name and parameters of the method.
    QString text;
    // If this is true, we don't show anything.
    bool silent;

    // The direct route. Every element of this will be shown as a line in the
    // inspectors of the corresponding (containing) module.
    PathVec path;
    // The graphical represenations of each "hop" in the path from srcMod to destMod.
    std::vector<std::map<ModuleInspector *, ConnectionItem *>> connectionItems;

    // The animation the body of which this one is.
    // Needed so we can walk up the tree in methodcallEnd.
    MethodcallAnimation *parent = nullptr;
    // The animations that have to be done while this methodcall "runs" (is visible).
    // These can be subcalls, or even sent messages, if they are completely instantaneous (in SimTime).
    AnimationSequence body;

public:
    MethodcallAnimation(cComponent *src, cComponent *dest, const char *text, bool silent);

    MethodcallAnimation *getParent() const { return parent; } // see parent
    // Appends an animation to the body of this methodcall animation.
    void addOperation(Animation *operation);

    void begin() override;
    void update() override;
    void end() override;

    // Recursive: Also adds the child animations (in the 'body' sequence) to insp.
    void addToInspector(Inspector *insp) override;
    void updateInInspector(Inspector *insp) override;
    void removeFromInspector(Inspector *insp) override;

    bool isEmpty() const override;
    bool willAnimate(cMessage *msg) override { return body.willAnimate(msg); }
    void messageDuplicated(cMessage *msg, cMessage *dup) override { body.messageDuplicated(msg, dup); }
    void removeMessagePointer(cMessage *msg) override { body.removeMessagePointer(msg); }

    QString str() const override;

    ~MethodcallAnimation() override;
};

class QTENV_API MessageAnimation : public Animation {
protected:
    // The actual cMessage used by the model.
    // This is used as long as possible, until we are asked
    // to release it (possibly because it was delivered or deleted).
    cMessage *msg = nullptr;
    // The privateDup of msg made by LogInspector.
    // This is filled in messageDuplicated, and takes the place of msg
    // in removeMessagePointer, when we are no longer allowed to use that.
    cMessage *msgDup = nullptr;

    // The graphical representations of the animated message.
    std::map<ModuleInspector *, MessageItem *> messageItems;

    explicit MessageAnimation(cMessage *msg, double holdDuration): Animation(holdDuration), msg(msg) { }
    explicit MessageAnimation(cMessage *msg): Animation(), msg(msg) { }

    // Returns the original message if we are still allowed
    // to use it, and the private copy of it if not.
    cMessage *msgToUse() const { return msg ? msg : msgDup; }

public:

    void begin() override;
    void end() override;

    bool isEmpty() const override { return messageItems.empty(); }
    bool willAnimate(cMessage *msg) override { return state < FINISHED && msg == this->msg; }
    void removeMessagePointer(cMessage *msg) override;
    void messageDuplicated(cMessage *msg, cMessage *dup) override;

    // call this from subclasses!
    void removeFromInspector(Inspector *insp) override;

    ~MessageAnimation() override;
};

// TODO: animate discarsion
class QTENV_API SendOnConnAnimation : public MessageAnimation
{
    cGate *gate; // source

    // when the first bit of the message is sent, the propagation delay and transmission duration.
    // 开始传播时间，传播的时延，以及持续时间
    SimTime start, prop, trans;

    // Where this hop should travel in the given inspector.
    QLineF getLine(ModuleInspector *mi);

public:
    // The holding variant with 0 delays.
    SendOnConnAnimation(cGate *gate, cMessage *msg)
        : MessageAnimation(msg, 1.0), gate(gate) { }

    // The non-holding variant with finite delay(s). TODO: animate discarsion
    SendOnConnAnimation(cGate *gate, cMessage *msg, SimTime start, SimTime prop, SimTime trans, bool discard)
        : MessageAnimation(msg), gate(gate), start(start), prop(prop), trans(trans) { }

    void begin() override;
    void update() override;

    void addToInspector(Inspector *insp) override;
    void updateInInspector(Inspector *insp) override;

    QString str() const override;

    int getSourceModuleId() override { return gate->getOwnerModule()->getId(); }
};

class QTENV_API SendDirectAnimation : public MessageAnimation
{
    int srcModuleId;
    cGate *dest;
    SimTime start, prop, trans;

    std::map<ModuleInspector *, ConnectionItem *> connectionItems;

public:
    SendDirectAnimation(cModule *src, cMessage *msg, cGate *dest);
    SendDirectAnimation(cModule *src, cMessage *msg, cGate *dest, SimTime start, SimTime prop, SimTime trans);

    void init() override;
    void begin() override;
    void update() override;
    void end() override;

    void addToInspector(Inspector *insp) override;
    void updateInInspector(Inspector *insp) override;
    void removeFromInspector(Inspector *insp) override;

    QString str() const override;

    int getSourceModuleId() override { return srcModuleId; }

    ~SendDirectAnimation() override;
};

class QTENV_API DeliveryAnimation : public MessageAnimation {
    cGate *gate = nullptr; // source gate. if nullptr, this is a deliveryDirect

    QLineF getLine(ModuleInspector *mi) const;

public:
    // for delivery on a connection
    explicit DeliveryAnimation(cGate *gate, cMessage *msg)
        : MessageAnimation(msg, 0.25), gate(gate) { }
    // for delivery after a sendDirect
    explicit DeliveryAnimation(cMessage *msg)
        : MessageAnimation(msg, 0.5) { }

    void begin() override;
    void update() override;

    void addToInspector(Inspector *insp) override;
    void updateInInspector(Inspector *insp) override;

    QString str() const override;

    int getSourceModuleId() override { return gate->getOwnerModule()->getId(); }
};

} // namespace qtenv
} // namespace omnetpp

#endif // __OMNETPP_QTENV_MESSAGEANIMATIONS_H
