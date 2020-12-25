//==========================================================================
//  displayupdatecontroller.h - part of
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

#ifndef __OMNETPP_QTENV_DISPLAYUPDATECONTROLLER_H
#define __OMNETPP_QTENV_DISPLAYUPDATECONTROLLER_H

#include <deque>

#include <QElapsedTimer>
#include <QToolButton>

#include "qtenv.h"

namespace omnetpp {
namespace qtenv {

class AnimationControllerDialog;

// 存储运行模式，动画播放等基础信息的配置文件
struct QTENV_API RunModeProfile {
    void save(const QString& prefix);
    void load(const QString& prefix);

    // these bound the value coming from the user/msganim, or NaN if not
    double minAnimationSpeed, maxAnimationSpeed;

    // the range of the slider on the toolbar
    double minPlaybackSpeed, maxPlaybackSpeed;

    // the current position of the slider
    double playbackSpeed;

    void setPlaybackSpeed(double speed) {
        playbackSpeed = clip(minPlaybackSpeed, speed, maxPlaybackSpeed);
    }
};

// 计时器，用来计算暂停了多久
class StopWatch {
    int64_t lostNSecs = 0;
    int64_t pausedNSecs = 0;
    int64_t pausedAt = 0;

    bool running = true;
    QElapsedTimer timer;
public:
    StopWatch() { timer.start(); }

    void reset() { timer.restart(); pausedNSecs = 0; pausedAt = 0; lostNSecs = 0; running = true; }

    void rewind(double secs) { lostNSecs += secs * 1.0e9; }

    void pause() { ASSERT(running); pausedAt = timer.nsecsElapsed(); running = false; }
    void resume() { ASSERT(!running); pausedNSecs += timer.nsecsElapsed() - pausedAt; running = true; }

    bool isRunning() { return running; }
    double getElapsedSeconds() { return ((running ? timer.nsecsElapsed() : pausedAt) - pausedNSecs - lostNSecs) / 1.0e9; }
    double getElapsedSecondsNoLoss() { return ((running ? timer.nsecsElapsed() : pausedAt) - pausedNSecs) / 1.0e9; }
};

class QTENV_API DisplayUpdateController : public QObject
{
    Q_OBJECT

    // 三胞胎：这里是指真实时间，动画时间，以及仿真时间的组合，但是这个类其实是存储了4个变量
    struct TimeTriplet {
        double realTime = 0; // can still have "pauses" for a few reasons
        double animationTime = 0;
        SimTime simTime = SimTime::ZERO;

        // animationSpeed * playbackSpeed recorder at the time of the frame,
        // only used for FPS/simSpeed measurement
        double effectiveAnimationSpeed = NAN;

        TimeTriplet() { }
        TimeTriplet(double rt, double at, SimTime st, double eas):
            realTime(rt), animationTime(at), simTime(st), effectiveAnimationSpeed(eas) { }

        QString str() {
            return QString("real: %1 anim: %2 sim: %3")
                    .arg(realTime).arg(animationTime).arg(simTime.str().c_str());
        }
    };

    // 这个变量用来存储
    TimeTriplet currentTimes;

    // 下面三个指针，都不是管理的关系，而是引用的快捷方式
    cSimulation *sim = getSimulation();
    Qtenv *qtenv = getQtenv();
    MessageAnimator *msgAnim = qtenv->getMessageAnimator();

    // 生成了一组默认值，这个结构体本身就是5个float的组合而已
    RunModeProfile runProfile  = {std::nan(""), std::nan(""), 1e-2, 1e2, 1};
    RunModeProfile fastProfile = {std::nan(""), std::nan(""), 1e0,  1e6, 1000};

    // 通过此指针，指向之前的两个配置文件，主窗口，滑动条的值就是写这里的第5个参数里，
    RunModeProfile *currentProfile = &runProfile;

    RunMode runMode = RUNMODE_NOT_RUNNING;

    AnimationControllerDialog *dialog = nullptr;

    double videoFps = 30; // 记录视频的帧率

    // if positive, just stop the progression of animationTime, but when zero, go on with whatever was
    // going on before - useful for when context menus, modal dialogs, data editors are shown, etc...
    // not a simple bool because "pausing" can happen because the simulation is not running, or when
    // an editor is open in the g.o.i. for example. Must not be negative.
    int pausedCount = 0;

    StopWatch stopwatch;

    const double minFrameRate = 2; // 最小帧率
    // 得保持GUI一直有反应，即便当我们没有真正的显示更新
    //refreshDisplay()，有时是我们滞后了
    const double minGuiUpdateRate = 10; // 每秒至少迭代几次事件

    double lastIntervalAt = 0;          // 前一次间隔
    double lastFrameAt = 0;             // 最后一帧
    double lastGuiUpdateAt = 0;         // 上次更新时间
    std::deque<TimeTriplet> frameTimes; // in lossless stopwatch time, to calculate fps

    bool recordingVideo = false;        // 状态标记

    int frameCount = 0;                 // 这个用来表示下一帧的记录的流水串号

    // used in rendering mode, this stores the last SimTime we recorded, incremented by constant amounts
    // 在rendering模式中有用，这里记录上次记录时间，按常量来计数
    simtime_t lastRecordedFrame = -SimTime::getMaxTime(); 

    std::string filenameBase = "frames/"; // 帧文件路径的前缀

    void renderFrame(bool record);       // 更新GUI的多个部分，体现当前的状态
    void recordFrame();    // 使用下一个串号来记录主界面的帧

    // 核心的2个函数就在这里
    // 在animateUntilNextEvent()中调用，如果recordingVideo是TRUE时
    // 如果真的到达了下一个事件的事件则返回true, 如果之前暂停了，崩溃了返回false
    bool renderUntilNextEvent(bool onlyHold); 
    // 在代理的renderUntilNextEvent()中调用 ，如果recordingVideo是TRUE时
    bool animateUntilNextEvent(bool onlyHold); 

    void advanceToSimTime(TimeTriplet &triplet, SimTime simTarget); // 不改动真实时间
    double advanceToRealTimeLimited(TimeTriplet &triplet, double realTarget, SimTime simLimit);

signals:
    void playbackSpeedChanged(double speed);
    void runModeChanged(RunMode mode);

public slots:
    void setPlaybackSpeed(double speed);
    void setPlaybackSpeed(double speed, RunModeProfile *profile);

    void pause() { if (pausedCount == 0) stopwatch.pause(); ++pausedCount; }
    void resume() { ASSERT(pausedCount > 0); --pausedCount; if (pausedCount == 0) stopwatch.resume(); }

public:
    DisplayUpdateController();

    double getCurrentFps() const;
    double getCurrentSimulationSpeed() const;
    bool effectiveAnimationSpeedRecentlyChanged() const;

    double getAnimationTime() const { return currentTimes.animationTime; }
    double getAnimationSpeed() const;
    double getAnimationHoldEndTime() const;

    void setRunMode(RunMode value);

    // returns true if really reached the time for the next event, false if interrupted/stopped before
    // 这两句其实都是先计算出某个时间，然后运行到那个时间点
    bool animateUntilNextEvent() { return animateUntilNextEvent(false); }
    bool animateUntilHoldEnds() { return animateUntilNextEvent(true); }

    void idle();
    void simulationEvent();

    void skipHold(); // sets animationTime to the end of the current hold, effectively ending it
    void skipToNextEvent(); // the above, plus sets simTime to that of the next event

    bool isPaused() { return pausedCount > 0; }

    // true if nothing (builtin anim, model hold, simtime anim) is "waiting to happen" before the next event
    bool rightBeforeEvent();

    void startVideoRecording() { recordingVideo = true; }
    void stopVideoRecording() { recordingVideo = false; }

    std::string getFilenameBase() const { return filenameBase; }
    void setFilenameBase(const char *base) { filenameBase = base; }

    double getVideoFps() const { return videoFps; }
    void setVideoFps(double value) { videoFps = value; }

    void showDialog(QAction *action);
    void hideDialog();

    RunModeProfile *getCurrentProfile() { return currentProfile; }
    RunModeProfile *getStepRunProfile() { return &runProfile; }
    RunModeProfile *getFastProfile() { return &fastProfile; }

    double getMinPlaybackSpeed() const { return currentProfile->minPlaybackSpeed; }
    double getMaxPlaybackSpeed() const { return currentProfile->maxPlaybackSpeed; }
    double getPlaybackSpeed() const { return currentProfile->playbackSpeed; }

    void setMinAnimSpeed(double value) { currentProfile->minAnimationSpeed = value; }
    void setMaxAnimSpeed(double value) { currentProfile->maxAnimationSpeed = value; }

    int getFrameCount() const { return frameCount; }

    void reset();

    ~DisplayUpdateController();
};

} // namespace qtenv
} // namespace omnetpp

#endif // __OMNETPP_QTENV_DISPLAYUPDATECONTROLLER_H
