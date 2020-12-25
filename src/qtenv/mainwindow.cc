//==========================================================================
//  MAINWINDOW.CC - part of
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>

#include <QMessageBox>
#include <QCloseEvent>
#include <QToolTip>
#include <QStyledItemDelegate>
#include <QTextLayout>
#include <QTimer>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QDebug>
#include <QToolButton>

#include "omnetpp/csimplemodule.h"
#include "omnetpp/csimulation.h"
#include "omnetpp/cfutureeventset.h"
#include "omnetpp/cscheduler.h"
#include "common/stringutil.h"
#include "common/ver.h"

#include "runselectiondialog.h"
#include "treeitemmodel.h"
#include "inspector.h"
#include "stopdialog.h"
#include "inspectorutil.h"
#include "genericobjectinspector.h"
#include "loginspector.h"
#include "timelineinspector.h"
#include "moduleinspector.h"
#include "timelinegraphicsview.h"
#include "rununtildialog.h"
#include "findobjectsdialog.h"
#include "comboselectiondialog.h"
#include "fileeditor.h"
#include "animationcontrollerdialog.h"
#include "displayupdatecontroller.h"
#include "videorecordingdialog.h"
#include "qtutil.h"

#define emit

using namespace omnetpp::common;

namespace omnetpp {
namespace qtenv {

QString MainWindow::aboutText = "xNeT++\n离散事件仿真套件-协议仿真 版本1.0.1 \n\n(C) 2003-2020 Huaru.com\n";


MainWindow::MainWindow(Qtenv *env, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    env(env)
{
    // mianwindows.ui编译后的ui_mainwindows.h中，设置界面基本元素
    // 不需要对h文件直接更改，而需要对UI文件编辑
    ui->setupUi(this);

    stopDialog = new StopDialog(this);  // 极速运行的停止按钮对话框
    fileEditor = new FileEditor(this);  // 打开INI文件编辑的那个小窗口编辑器

    // This will hold the toolbar itself, the animation speed
    // slider (the slider can't be allowed to collapse, since it
    // can't be inserted into a menu), then a stretch, and
    // the two event labels at the right.
    // 这里用了一个水平布局，放置工具栏，动画速度条，2个消息LABEL
    auto toolBarLayout = new QHBoxLayout(ui->toolBarArea);
    toolBarLayout->setMargin(0);

    // clearing the hack notification, so we won't be caught
    ui->toolBarArea->setWhatsThis("");

    // This is a HACK, but this will reparent the main toolbar
    // into this widget without any problems.
    // It is necessary to make the event labels always visible and
    // start collapsing the action icons when the window becomes
    // too narrow instead of the more important labels on the toolbar.
    // 步骤1：将工具条添加到布局中，这里之所以用了HACK一词，是因为本来工具条在
    // 界面设计时候已经添加好了，但是这里中间偷偷加了一层水平布局管理。
    toolBarLayout->addWidget(ui->mainToolBar);

    // This is the animation playback speed slider on the toolbar.
    // It is exponential, see the playbackSpeedToSliderValue and
    // sliderValueToPlaybackSpeed functions for the mapping.
    // A simple snapping mechanism is also implemented in
    // onSliderValueChanged to make selecting 1.0 easier
    slider = new QSlider();
    slider->setOrientation(Qt::Orientation::Horizontal);
    slider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    slider->setTracking(true);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setMinimumWidth(180);
    slider->setMaximumHeight(ui->mainToolBar->height());
    // 步骤2：将速度条添加到布局中
    toolBarLayout->addWidget(slider);
    // this will set it up with the current limits
    setGuiForRunmode(env->getSimulationRunMode());

    // add current event status
    simTimeLabel = new QLabel();
    simTimeLabel->setToolTip("当前仿真时间");
    simTimeLabel->setFrameStyle(ui->nextModuleLabel->frameStyle());
    simTimeLabel->setObjectName("simTimeLabel");
    simTimeLabel->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(simTimeLabel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onSimTimeLabelContextMenuRequested(QPoint)));

    eventNumLabel = new QLabel();
    eventNumLabel->setToolTip("事件号");
    eventNumLabel->setFrameStyle(ui->nextModuleLabel->frameStyle());
    eventNumLabel->setAlignment(Qt::Alignment(Qt::AlignVCenter | Qt::AlignRight));
    eventNumLabel->setObjectName("eventNumLabel");
    eventNumLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(eventNumLabel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onEventNumLabelContextMenuRequested(QPoint)));

    // this little widget will add a small margin above the two labels
    // 步骤3.0：消息控件的容器: 添加1个事件号，添加1个时间戳
    QWidget *labelsContainer = new QWidget();
    QHBoxLayout *labelsLayout = new QHBoxLayout();
    labelsLayout->setMargin(0);
    labelsContainer->setLayout(labelsLayout);

    labelsLayout->addWidget(eventNumLabel);
    eventNumLabel->setMinimumWidth(100);   // 不能太短，否则显示不全
    eventNumLabel->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    labelsLayout->addWidget(simTimeLabel);
    simTimeLabel->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));

    auto margin = centralWidget()->layout()->contentsMargins().right();
    labelsContainer->setContentsMargins(0, margin, 0, 0);

    // 步骤3.1：工具条位置添加一个弹性可伸缩空间，添加消息控件的容器
    toolBarLayout->addStretch(1);
    toolBarLayout->addWidget(labelsContainer);

    connect(env->getDisplayUpdateController(), &DisplayUpdateController::playbackSpeedChanged,
            this, &MainWindow::updateSpeedSlider);

    adjustSize();

    ui->actionDebugOnErrors->setChecked(getQtenv()->debugOnErrors);

    // robin added
    //QMenu * pTest = ui->menuBar->addMenu(QString::fromLocal8Bit("测试"));
    //QAction *action = pTest->addAction("设置位置", this, SLOT(on_actionPos));
    //on_actionPos();
    connect(ui->actionPos, SIGNAL(triggered()), this, SLOT(on_actionPos()));
    connect(ui->actionDown, SIGNAL(triggered()), this, SLOT(on_actionDown()));
    connect(ui->actionUp, SIGNAL(triggered()), this, SLOT(on_actionUp()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete fileEditor;
    delete stopDialog;
    delete simTimeLabel;
    delete eventNumLabel;
}

void MainWindow::onSimTimeLabelGroupingTriggered()
{
    QVariant variant = static_cast<QAction *>(sender())->data();

    simTimeDigitGrouping = (DigitGrouping)variant.value<int>();
    updateSimTimeLabel();
}

void MainWindow::onSimTimeLabelUnitsTriggered()
{
    simTimeUnits = !simTimeUnits;
    updateSimTimeLabel();
}

void MainWindow::updateSimTimeLabel()
{
    const char *digitSeparator, *decimalSeparator = ".";

    switch (simTimeDigitGrouping) {
        case SPACE:
            digitSeparator = "<small> </small> ";
            decimalSeparator = ".";
            break;
        case COMMA:
            digitSeparator = ",";
            break;
        case APOSTROPHE:
            digitSeparator = "'";
            break;
        case NONE:
            digitSeparator = "";
            break;
    }

    QString simTimeText;
    if (simTimeUnits)
        simTimeText = getSimulation()->getSimTime().format(SimTime::getScaleExp(), decimalSeparator, digitSeparator, 
            true, "<small><font color=grey>", " </font></small>").c_str();
    else
        simTimeText = getSimulation()->getSimTime().format(SimTime::getScaleExp(), decimalSeparator, digitSeparator).c_str();

    simTimeLabel->setText(simTimeText);
}

// 工具栏位置那个时间控件，右键，会有菜单
void MainWindow::onSimTimeLabelContextMenuRequested(QPoint pos)
{
    QMenu *menu = new QMenu();

    QActionGroup *actionGroup = new QActionGroup(menu);

    //TODO use this once it compiles everywere:
    //QVector<QPair<QString, int>> elements = {
    //    {"No digit grouping", NONE},
    //    {"Group digits with comma", COMMA}
    //    {"Group digits with apostrophe", APOSTROPHE},
    //    {"Group digits with space", SPACE}
    //};
    QVector<QPair<QString, int>> elements;
    elements.push_back(QPair<QString, int>("不分节&N", NONE));
    elements.push_back(QPair<QString, int>("逗号分节&C", COMMA));
    elements.push_back(QPair<QString, int>("单引号分节&A", APOSTROPHE));  // &Apostrophe
    elements.push_back(QPair<QString, int>("空格分节&S", SPACE));

    QAction *action;
    for (auto elem : elements) {
        action = menu->addAction(elem.first, this, SLOT(onSimTimeLabelGroupingTriggered()));
        action->setData(QVariant::fromValue(elem.second));
        action->setActionGroup(actionGroup);
        action->setCheckable(true);
        if (simTimeDigitGrouping == elem.second)
            action->setChecked(true);
    }
    menu->addSeparator();

    action = menu->addAction("显示单位&D", this, SLOT(onSimTimeLabelUnitsTriggered()));
    action->setCheckable(true);
    action->setChecked(simTimeUnits);

    menu->exec(simTimeLabel->mapToGlobal(pos));

    delete menu;
}
// 更新事件号
void MainWindow::updateEventNumLabel()
{
    const char *digitSeparator;

    switch (eventNumDigitGrouping) {
        case SPACE:
            digitSeparator = "<small> </small> ";
            break;
        case COMMA:
            digitSeparator = ",";
            break;
        case APOSTROPHE:
            digitSeparator = "'";
            break;
        case NONE:
            digitSeparator = "";
            break;
    }

    // 其实应该是BOOL变量，显示下一个事件，或者当前事件
    double showNextEvent = env->getDisplayUpdateController()->rightBeforeEvent() && env->getSimulationRunMode() == RUNMODE_NOT_RUNNING;

    //  显示下一事件，则需要加1
    eventnumber_t numToShow = getSimulation()->getEventNumber() + (showNextEvent ? 1 : 0);
    const char *prefix = showNextEvent ? "当前: " : "下一个: ";

    // arg函数是用来将格式化字符串中的%1开始的占位符替换为参数
    QString eventNumText = QString("<font color=grey><small>%1</small>#</font>").arg(prefix)

            // 在英文中，长的十进制数字，按照3位一个分组，这里进行分割
            + opp_format(numToShow, digitSeparator).c_str();

    eventNumLabel->setText(eventNumText);
    eventNumLabel->setStyleSheet(showNextEvent ? "" : "QLabel { background: palette(alternate-base); }");
}

void MainWindow::onEventNumLabelGroupingTriggered()
{
    QVariant variant = static_cast<QAction *>(sender())->data();

    eventNumDigitGrouping = (DigitGrouping)variant.value<int>();
    updateEventNumLabel();
}
// 工具栏位置那个事件号控件，右键单击，会有菜单；这个函数是创建菜单
void MainWindow::onEventNumLabelContextMenuRequested(QPoint pos)
{
    QMenu *menu = new QMenu();

    QActionGroup *actionGroup = new QActionGroup(menu);

    QVector<QPair<QString, int>> elements = {
        { "不分节&N", NONE },
        { "逗号分节&C", COMMA },
        { "单引号分节&A", APOSTROPHE },
        { "空格分节&S", SPACE }
    };

    QAction *action;
    for (auto elem : elements) {
        action = menu->addAction(elem.first, this, SLOT(onEventNumLabelGroupingTriggered()));
        action->setData(elem.second);
        action->setActionGroup(actionGroup);
        action->setCheckable(true);
        if (eventNumDigitGrouping == elem.second)
            action->setChecked(true);
    }

    menu->exec(eventNumLabel->mapToGlobal(pos));

    delete menu;
}

bool MainWindow::isRunning()
{
    Qtenv::eState state = (Qtenv::eState)env->getSimulationState();
    return state == Qtenv::SIM_RUNNING || state == Qtenv::SIM_BUSY;
}

// 根据当前的状态，设置按钮的状态，
void MainWindow::setGuiForRunmode(RunMode runMode, bool untilMode)
{
    ui->actionOneStep->setChecked(runMode == RUNMODE_STEP);
    ui->actionRun->setChecked(runMode == RUNMODE_NORMAL);
    ui->actionFastRun->setChecked(runMode == RUNMODE_FAST);
    ui->actionExpressRun->setChecked(runMode == RUNMODE_EXPRESS);

    if (runMode == RUNMODE_NOT_RUNNING)
        ui->actionRunUntil->setChecked(false);

    if (runMode == RUNMODE_EXPRESS)
        showStopDialog();
    else
        closeStopDialog();

    auto duc = env->getDisplayUpdateController();
    duc->setRunMode(runMode);

    //slider->setEnabled(runMode != RUNMODE_NOT_RUNNING);

    ui->actionRunUntil->setChecked(untilMode);
}

void MainWindow::showStopDialog()
{
    stopDialog->show();
    setEnabled(false);
    stopDialog->setEnabled(true);
    QApplication::processEvents();  // so the dialog will show up immediately
}

void MainWindow::closeStopDialog()
{
    if (stopDialog->isVisible())
        stopDialog->close();

    setEnabled(true);
}

void MainWindow::enterLayoutingMode()
{
    ASSERT(disabledForLayouting.empty());

    for (auto c : findChildren<QObject *>()) {
        auto action = dynamic_cast<QAction *>(c);
        if (action && action->isEnabled() && action != ui->actionStop) {
            disabledForLayouting.insert(action);
            action->setEnabled(false);
        }

        auto inspector = dynamic_cast<Inspector *>(c);
        if (inspector && inspector->isEnabled()) {
            disabledForLayouting.insert(inspector);
            inspector->setEnabled(false);
        }
    }

    for (auto c : ui->toolBarArea->findChildren<QObject *>()) {
        auto widget = dynamic_cast<QWidget*>(c);
        if (widget && widget->isEnabled() && !widget->actions().contains(ui->actionStop)) {
            disabledForLayouting.insert(widget);
            widget->setEnabled(false);
        }
    }
}

void MainWindow::exitLayoutingMode()
{
    for (auto c : disabledForLayouting) {
        if (auto action = dynamic_cast<QAction *>(c))
            action->setEnabled(true);

        if (auto inspector = dynamic_cast<Inspector *>(c))
            inspector->setEnabled(true);

        if (auto widget = dynamic_cast<QWidget *>(c))
            widget->setEnabled(true);
    }

    disabledForLayouting.clear();
}

// 这个函数是从控制器，将速度映射到滑动条，
// 当联合仿真，收到控制命令时候，也需要调用这个函数
void MainWindow::updateSpeedSlider()
{
    auto duc = env->getDisplayUpdateController();
    bool blocked = slider->blockSignals(true);
    int min = playbackSpeedToSliderValue(duc->getMinPlaybackSpeed());
    int max = playbackSpeedToSliderValue(duc->getMaxPlaybackSpeed());
    slider->setMinimum(min);
    slider->setMaximum(max);                   // 设置范围
    slider->setPageStep((max - min) / 25);     // 步长
    slider->setSingleStep((max - min) / 50);   // 步长

    // 刻度间隔是100，取值范围是-200   0   200，所以初始化之后是5个刻度线
    slider->setTickInterval(playbackSpeedToSliderValue(10) - playbackSpeedToSliderValue(1));

    slider->setValue(playbackSpeedToSliderValue(duc->getPlaybackSpeed()));
    slider->setToolTip("播放速度: " + QString::number(duc->getPlaybackSpeed(), 'f', 4));
    slider->blockSignals(blocked);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Qtenv::eState state = env->getSimulationState();

    enum Action {
        QUIT, // no dialog, no finish, quit
        FINISH_QUIT, // no dialog, finish, quit
        YES_NO_CANCEL, // yes/no/cancel dialog, maybe (maybe finish, quit)
    };

    Action action = QUIT;

    bool confirmExit = env->getPref("confirm-exit", true).toBool();

    // First, deciding what to do
    switch (state) {
        case Qtenv::SIM_NONET: // if there is no network, we simply quit
        case Qtenv::SIM_NEW: // if there's a network, but not started, always exiting
            action = QUIT;
            break;

        case Qtenv::SIM_READY: // during simulation (running or paused), either ask to finish, or just do it
        case Qtenv::SIM_RUNNING:
        case Qtenv::SIM_BUSY:
            action = confirmExit ? YES_NO_CANCEL : FINISH_QUIT;
            break;

        case Qtenv::SIM_TERMINATED: // <- this can't happen by the way, we always finish() after termination right away
        case Qtenv::SIM_FINISHCALLED: // if the simulation ended properly, a simple confirmation or nothing
            action = QUIT;
            break;

        case Qtenv::SIM_ERROR: // after an error, do not ask for confirmation [Andras]
            action = QUIT;
            break;
    }

    // Then acting on our decision:
    switch (action) {
        case QUIT:
            // nothing to do
            break;
        case FINISH_QUIT:
            ASSERT(getQtenv()->getCallFinishOnExitFlag() == false);
            getQtenv()->setCallFinishOnExitFlag(true);
            break;
        case YES_NO_CANCEL: {
            QString question3 = "退出前你要调用finish()函数对仿真总结一下么？";
            QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;

            int ans = QMessageBox::question(this, "提示", question3, buttons, QMessageBox::Yes);

            if (ans == QMessageBox::Yes) {
                ASSERT(getQtenv()->getCallFinishOnExitFlag() == false);
                getQtenv()->setCallFinishOnExitFlag(true);
            }

            if (ans == QMessageBox::Cancel) {
                event->ignore();
                return;
            }
            break;
        }
    }

    // finally letting it go, if we got here anyway
    env->setStopSimulationFlag();
    QMainWindow::closeEvent(event);
    emit closed();
}

// 这是在Mainwindows中一个总的入口函数，
void MainWindow::runSimulation(RunMode runMode)
{
    if (isRunning()) {
        setGuiForRunmode(runMode);
        env->setSimulationRunMode(runMode);
        setRunUntilModule();
    }
    else {
        if (!networkReady()) {
            setGuiForRunmode(RUNMODE_NOT_RUNNING);
            return;
        }
        setGuiForRunmode(runMode);
        // 核心是调用这句，这个是入口
        env->runSimulation(runMode);
        setGuiForRunmode(RUNMODE_NOT_RUNNING);
        closeStopDialog();
    }
}

// 如果重新选择了某个配置，则需要重新设置运行
void MainWindow::on_actionSetUpConfiguration_triggered()
{
    if (env->checkRunning())
        return;

    cConfigurationEx *configEx = getQtenv()->getConfigEx();

    // No filter used for subsequent run selections.
    // Note that if invoked this way, we pretty much avoid all possibility of an exception,
    // because the run filter is constant, and the config name is the current one, so it must exist.
    // This, and the fact that Qtenv::displayException is protected, along with Qt not supporting
    // throwing exceptions from slots, justifies the omission of a try-catch block.
    // It would only be an ASSERT(false) or something similar anyway.
    RunSelectionDialog dialog(configEx, configEx->getActiveConfigName(), "", this);
    if (dialog.exec()) {
        busy("正在建立新的运行");
        emit setNewNetwork();
        env->newRun(dialog.getConfigName().c_str(), dialog.getRunNumber());
        busy();
        reflectRecordEventlog();
    }
}

// stopSimulation
void MainWindow::on_actionStop_triggered()
{
    // implements Simulate|Stop
    if (env->getSimulationState() == Qtenv::SIM_RUNNING || env->getSimulationState() == Qtenv::SIM_BUSY) {
        // "opp_stopsimulation" just *asks* the simulation to stop, causing it to return
        // from the "opp_run" command.
        // "setGuiForRunmode notrunning" will be called after "opp_run" has returned.
        env->setStopSimulationFlag();
    }

    closeStopDialog();

    // this proc doubles as "stop layouting", when in graphical module inspectors
    // TODO
    // global stoplayouting
    // set stoplayouting 1
}

// runUntil 运行到某个命令或者位置
void MainWindow::on_actionRunUntil_triggered()
{
    // implements Simulate|Run until...
    if (!networkReady()) {
        setGuiForRunmode(RunMode::RUNMODE_NOT_RUNNING);
        return;
    }

    RunUntilDialog runUntilDialog;
    if (!runUntilDialog.exec()) {
        // popping the button back out
        setGuiForRunmode(getQtenv()->getSimulationRunMode());
        return;
    }

    RunMode runMode = runUntilDialog.getMode();
    simtime_t time = runUntilDialog.getTime();
    eventnumber_t event = runUntilDialog.getEventNumber();
    cMessage *msg = static_cast<cMessage *>(runUntilDialog.getMessage());
    bool stopOnMsgCancel = runUntilDialog.stopOnMsgCancel();

    bool untilMode = time.dbl() != 0 || event != 0 || msg != nullptr;
    if (isRunning()) {
        setGuiForRunmode(runMode, untilMode);
        getQtenv()->setSimulationRunMode(runMode);
        getQtenv()->setSimulationRunUntil(time, event, msg, stopOnMsgCancel);
    }
    else {
        if (!networkReady())
            return;

        setGuiForRunmode(runMode, untilMode);
        getQtenv()->runSimulation(runMode, time, event, msg, nullptr, stopOnMsgCancel);
        setGuiForRunmode(RUNMODE_NOT_RUNNING);
        closeStopDialog();
    }
}

// 当速度条被鼠标拖动而发生改变的时候，触发此函数
void MainWindow::onSliderValueChanged(int value)
{
    // 计算一个映射的速度，这里算法是值除以100，作为10的幂数
    double speed = sliderValueToPlaybackSpeed(value);

    // 当鼠标拖动时候，如果距离太小，就吸附到速度1.0上，也就是中间位置
    if (slider->isSliderDown()) {
        int pixelThreshold = 2;
        double snapToSpeed = 1.0;

        // finding out the length in pixels the slider handle can travel (approx)
        QStyleOptionSlider opt;
        opt.initFrom(slider);
        int span = slider->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this).width()
                - slider->style()->pixelMetric(QStyle::PM_SliderLength);

        // 反向做10的对数，再乘100，这个得到的值是滑块中间位置，
        int valueOne = playbackSpeedToSliderValue(snapToSpeed);

        // 如果之前的位置与当前位置的像素差，小于阈值的2个像素，则需要
        int positionOne = QStyle::sliderPositionFromValue(slider->minimum(), slider->maximum(), valueOne, span);
        int positionCurrent = QStyle::sliderPositionFromValue(slider->minimum(), slider->maximum(), value, span);

        if (std::abs(positionOne - positionCurrent) <= pixelThreshold) 
        {
            slider->setValue(valueOne);
            speed = snapToSpeed;  

        }

        // 这里注释了，主要是永远是0，BUG
       // if (env->getAnimationSpeed() == 0)

        if (speed == 0)
            QToolTip::showText(QCursor::pos(), "回放速度值为0: 请重新设置动画速度!", slider);
        else
        {
            QString str = QString("回放速度值为%1").arg(speed);
            QToolTip::showText(QCursor::pos(), str, slider);
        }
        
    } // end if 
    // 最后设置tooltip，可以显示当前的速度值
    slider->setToolTip(QString::number(speed, 'f', 4));

    // 这里重新设置控制器的速度
    env->getDisplayUpdateController()->setPlaybackSpeed(speed);
}

// 更新顶部状态栏时候，区分运行态与结束时态
void MainWindow::updateStatusDisplay()
{
    updateSimtimeDisplay();

    if (showStatusDetails) {
        //FIXME: switching between NextEvent Display and Performance Display should be explicit and in the Qtenv class not here! --Andras
        if (env->getSimulationState() == Qtenv::SIM_RUNNING
                && (env->getSimulationRunMode() == RUNMODE_FAST
                    || env->getSimulationRunMode() == RUNMODE_EXPRESS))
            updatePerformanceDisplay();
        else
            updateNextEventDisplay();
    }
}

// 这个函数负责通知更新上面的事件号，时间，下面状态栏的信息
void MainWindow::updateSimtimeDisplay()
{
    updateEventNumLabel();
    updateSimTimeLabel();
    ui->labelMessageStats->setText("消息状态: " + QString::number(getSimulation()->getFES()->getLength())
            +" 排队 / " + QString::number(cMessage::getLiveMessageCount())
            +" 存在 / " + QString::number(cMessage::getTotalMessageCount()) + " 已创建");
}

// 结束后显示：顶部状态栏显示的3个信息，
void MainWindow::updatePerformanceDisplay()
{
    //FIXME FIXME FIXME: should NOT overload the widgets used by updateNextEventDisplay()! Should use SEPARATE widgets not nextModuleLabel/nextEventLabel/nextTimeLabel, and swap the two statusbars when needed! Also: move away tooltip setting, into some initialization code! --Andras

    // Set Status Detail's tooltips
    ui->nextEventLabel->setToolTip("性能: 每秒处理的事件数目");
    ui->nextModuleLabel->setToolTip("相对速度: 每秒处理的仿真秒数");
    ui->nextTimeLabel->setToolTip("事件密度: 每仿真秒的事件数");

    // Set Status Detail's texts
    ui->nextModuleLabel->setText("仿真秒/秒: " + QString::number(env->getSpeedometer().getSimSecPerSec()));
    ui->nextEventLabel->setText("事件/秒: " + QString::number(env->getSpeedometer().getEventsPerSec()));
    ui->nextTimeLabel->setText("事件/仿真秒: " + QString::number(env->getSpeedometer().getEventsPerSimSec()));
}

// 运行状态，顶部状态栏显示的3个信息，
void MainWindow::updateNextEventDisplay()
{
    // Set Status Detail's tooltips
    ui->nextEventLabel->setToolTip("下一仿真事件");
    ui->nextModuleLabel->setToolTip("下一事件位于模块");
    ui->nextTimeLabel->setToolTip("下一事件仿真时间");

    cSimpleModule *modptr = nullptr;
    cEvent *msgptr = nullptr;

    int state = env->getSimulationState();
    if (state == Qtenv::SIM_NEW || state == Qtenv::SIM_READY || state == Qtenv::SIM_RUNNING || state == Qtenv::SIM_BUSY) {
        modptr = getSimulation()->guessNextModule();
        msgptr = getSimulation()->guessNextEvent();
    }

    // Set Status Detail's texts
    if (msgptr) {
        long objId = getObjectId(msgptr);
        simtime_t nextTime = getSimulation()->guessNextEvent()->getArrivalTime();
        simtime_t diff = nextTime - getSimulation()->getSimTime();
        ui->nextEventLabel->setText(QString("下一个: ") + msgptr->getName() + " (" + msgptr->getClassName()
                +", id=" + (objId == -1 ? "" : QString::number(objId)) + ")");
        ui->nextTimeLabel->setText(QString("排队于: ") + nextTime.str().c_str() + "s (当前+" + diff.str().c_str() + "s)");
    }
    else {
        ui->nextEventLabel->setText("下一个: n/a");
        ui->nextTimeLabel->setText("排队于: n/a");
    }

    if (modptr)
        ui->nextModuleLabel->setText(QString("隶属模型: ") + modptr->getFullPath().c_str()
                +" (" + getObjectShortTypeName(modptr) + ", id="
                + QString::number(modptr->getId()) + ")");
    else
        ui->nextModuleLabel->setText("隶属模型: n/a");
}

// 状态栏，左下角位置显示当前的配置信息
void MainWindow::updateNetworkRunDisplay()
{
    const char *configName = opp_nulltoempty(env->getConfigEx()->getActiveConfigName());
    const char *network = !getSimulation()->getNetworkType() ? "" : getSimulation()->getNetworkType()->getName();
    std::string scheduler = getSimulation()->getScheduler()->str();
    const char *sep = scheduler.empty() ? "" : " - ";

    // TODO
//    if {$configname==""} {set configName "n/a"}
//    if {$network==""} {set network "(no network)"}
    ui->labelConfigName->setText("配置项：" + QString(configName) + "，运行次数：" + 
        QString::number(env->getConfigEx()->getActiveRunNumber()) + "，网络类型： " + network + sep + scheduler.c_str());
}

void MainWindow::excludeMessageFromAnimation(cObject *msg)
{
    QString namePattern = msg->getFullName();
    namePattern.replace(QRegularExpression("[0-9]+"), "*"); // replace indices and such with '*' - hence the "messages like this" meaning
    namePattern.replace(QRegularExpression("[^[:print:]]"), "?");  // sanitize: replace nonprintable chars with '?'
    namePattern.replace(QRegularExpression("[\"\\\\]"), "?");  // sanitize: replace quotes (") and backslashes with '?'
    if (namePattern.contains(' '))  // must be quoted if contains spaces
        namePattern = "\"" + namePattern + "\"";

    QString filters = env->getSilentEventFilters();
    filters = filters.trimmed();
    if (!filters.isEmpty())
        filters += "\n";
    filters += namePattern + " and className(" + getObjectShortTypeName(msg, STRIPNAMESPACE_NONE) + ")\n";
    env->setSilentEventFilters(filters.toStdString().c_str());

    env->callRefreshInspectors();
}

void MainWindow::runUntilMsg(cMessage *msg, RunMode runMode)
{
    if (!networkReady())
        return;

    // mode must be "normal", "fast" or "express"
    if (isRunning()) {
        setGuiForRunmode(runMode, true);
        env->setSimulationRunMode(runMode);
        env->setSimulationRunUntil(SIMTIME_ZERO, 0, msg);
    }
    else {
        setGuiForRunmode(runMode, true);
        env->runSimulation(runMode, SIMTIME_ZERO, 0, msg);
        setGuiForRunmode(RUNMODE_NOT_RUNNING);
    }
}

// opp_set_run_until_module
void MainWindow::setRunUntilModule(Inspector *insp)
{
    if (insp == nullptr) {
        env->setSimulationRunUntilModule(nullptr);
        return;
    }

    cObject *object = insp->getObject();
    if (!object) {
        // TODO log "inspector has no object"
        return;
    }

    cModule *mod = dynamic_cast<cModule *>(object);
    if (!mod) {
        // TODO log "object is not a module"
        return;
    }

    env->setSimulationRunUntilModule(mod);
}

bool MainWindow::networkReady()
{
    if (!networkPresent())
        return false;

    if (isSimulationOk())
        return true;
    else {
        int ans = QMessageBox::warning(this, tr("告警"), tr("无法继续，是否重置网络?"),
                    QMessageBox::Yes | QMessageBox::No);
        if (ans == QMessageBox::Yes) {
            on_actionRebuildNetwork_triggered();
            return isSimulationOk();
        }
        else
            return false;
    }

    return true;
}

bool MainWindow::isSimulationOk()
{
    int state = env->getSimulationState();
    return state == Qtenv::SIM_NEW || state == Qtenv::SIM_RUNNING || state == Qtenv::SIM_READY;
}

bool MainWindow::networkPresent()
{
    if (!getSimulation()->getSystemModule()) {
        QMessageBox::warning(this, tr("错误"), tr("网络没有创建."), QMessageBox::Ok);
        return false;
    }
    return true;
}

void MainWindow::saveSplitter(QString prefName, QSplitter *splitter)
{
    QList<QVariant> sizes;
    sizes.clear();
    sizes.append(splitter->sizes()[0]);
    sizes.append(splitter->sizes()[1]);
    env->setPref(prefName, sizes);
}

void MainWindow::storeGeometry()
{
    env->setPref("mainwindow-geom", geometry());

    if (ui->actionTimeline->isChecked())
        saveSplitter("mainwin-main-splittersizes", ui->mainSplitter);
    else {
        QList<QVariant> sizes;
        sizes.clear();
        sizes.append(timeLineSize[0]);
        sizes.append(timeLineSize[1]);
        env->setPref("mainwin-main-splittersizes", sizes);
    }

    saveSplitter("mainwin-bottom-splittersizes", ui->splitter_3);
    saveSplitter("mainwin-left-splittersizes", ui->splitter);

    if (ui->splitter_2->orientation() == Qt::Horizontal) {
        saveSplitter("mainwin-right-splittersizes-horiz", ui->splitter_2);
        env->setPref("mainwin-right-splitter-orientation", "horiz");
    }
    else {
        saveSplitter("mainwin-right-splittersizes-vert", ui->splitter_2);
        env->setPref("mainwin-right-splitter-orientation", "vert");
    }

    getQtenv()->setPref("display-timeline", ui->actionTimeline->isChecked());
    getQtenv()->setPref("display-statusdetails", showStatusDetails);

    getQtenv()->setPref("simtimelabel-digitgrouping", simTimeDigitGrouping);
    getQtenv()->setPref("simtimelabel-units", simTimeUnits);

    getQtenv()->setPref("eventnumlabel-digitgrouping", eventNumDigitGrouping);
}

void MainWindow::restoreSplitter(QString prefName, QSplitter *splitter, const QList<int>& defaultSizes)
{
    QList<QVariant> sizes = env->getPref(prefName).value<QList<QVariant> >();
    QList<int> intSizes;

    if (sizes.size() >= 2) {
        intSizes.append(sizes[0].toInt());
        intSizes.append(sizes[1].toInt());
    }
    else {
        intSizes = defaultSizes;
    }

    if (intSizes.size() >= 2) {
        splitter->setSizes(intSizes);
    }
}

// 在Qtenv::doRun()中调用，窗口显示后，加载各个分隔条的位置信息
void MainWindow::restoreGeometry()
{
    QRect geom = env->getPref("mainwindow-geom").toRect();
    if (geom.isValid())
        setGeometry(geom);

    // set timeline initial size if there is no qtenv.ini
    QList<int> sizes;
    TimeLineInspector *timeLineInsp = static_cast<TimeLineInspector *>(ui->timeLine->children()[0]);
    connect(ui->mainSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(onSplitterMoved(int, int)));
    double timeLineHeight = timeLineInsp->getInitHeight();
    sizes.append(timeLineHeight);
    sizes.append(ui->mainSplitter->height() - timeLineHeight);
    defaultTimeLineSize = sizes;

    QList<int> defaultSizes;
    defaultSizes.append(ui->splitter_3->width() / 4);
    defaultSizes.append(ui->splitter_3->width() / 4 * 3);

    restoreSplitter("mainwin-main-splittersizes", ui->mainSplitter, sizes);        // 窗口主要功能区，用它分为上下，上部是时间轴，下部splitter_3
    restoreSplitter("mainwin-bottom-splittersizes", ui->splitter_3, defaultSizes); // 上部是时间轴，下部就是它
    restoreSplitter("mainwin-left-splittersizes", ui->splitter);                   // ui->splitter_3左侧是ui->splitter，分割对象列表和树视图

    QVariant orient = env->getPref("mainwin-right-splitter-orientation");          // 右侧上下，或者左右，显示网络与日志

    defaultSizes.clear();

    if (orient == "horiz") {
        ui->splitter_2->setOrientation(Qt::Horizontal);
        ui->actionHorizontalLayout->setChecked(true);
        defaultSizes.append(ui->splitter_2->width() / 4 * 3);
        defaultSizes.append(ui->splitter_2->width() / 4);
        restoreSplitter("mainwin-right-splittersizes-horiz", ui->splitter_2, defaultSizes);
    }
    else if (orient == "vert") {
        ui->splitter_2->setOrientation(Qt::Vertical);
        ui->actionVerticalLayout->setChecked(true);
        defaultSizes.append(ui->splitter_2->height() / 4 * 3);
        defaultSizes.append(ui->splitter_2->height() / 4);
        restoreSplitter("mainwin-right-splittersizes-vert", ui->splitter_2, defaultSizes);
    }

    // initialize timeline
    bool isSunken = getQtenv()->getPref("display-timeline", true).value<bool>();
    timeLineSize = ui->mainSplitter->sizes();
    on_actionTimeline_toggled(isSunken);
    ui->actionTimeline->setChecked(isSunken);

    // initialize status bar
    showStatusDetails = getQtenv()->getPref("display-statusdetails", true).value<bool>();
    ui->actionStatusDetails->setChecked(showStatusDetails);
    ui->nextEventLabel->setVisible(showStatusDetails);
    ui->nextModuleLabel->setVisible(showStatusDetails);
    ui->nextTimeLabel->setVisible(showStatusDetails);

    // initialize simTimeLabel
    simTimeDigitGrouping = (DigitGrouping)getQtenv()->getPref("simtimelabel-digitgrouping", int(APOSTROPHE)).value<int>();
    simTimeUnits = getQtenv()->getPref("simtimelabel-units", true).value<bool>();

    // initialize eventNumLabel
    eventNumDigitGrouping = (DigitGrouping)getQtenv()->getPref("eventnumlabel-digitgrouping", int(APOSTROPHE)).value<int>();
}

// rebuild
void MainWindow::on_actionRebuildNetwork_triggered()
{
    // implements Simulate|Rebuild

    if (env->checkRunning())
        return;

    if (!networkPresent())
        return;

    busy("正在重置network中...");
    // TODO inspectorList:addAll 1
    env->rebuildSim();
    reflectRecordEventlog();
    busy();
}

// XXX The message is ignored now that the status bar got rid of
void MainWindow::busy(QString msg)
{
    setCursor(QCursor(msg.isEmpty() ? Qt::ArrowCursor : Qt::WaitCursor));
}

void MainWindow::on_actionPreferences_triggered()
{
    InspectorUtil::preferencesDialog();
    getQtenv()->callRefreshInspectors();
}

void MainWindow::on_actionTimeline_toggled(bool isSunken)
{
    if (!isSunken) {
        timeLineSize = ui->mainSplitter->sizes().at(0) == 0 ? defaultTimeLineSize : ui->mainSplitter->sizes();
        QList<int> sizes = timeLineSize;
        sizes[0] = 0;
        ui->mainSplitter->setSizes(sizes);
    }
    else
        ui->mainSplitter->setSizes(timeLineSize);
}

void MainWindow::onSplitterMoved(int, int)
{
    // It is needed in case when hide timeline with toolbar button and after
    // is displayed by splitterMoved signal, thus it can be avoided glint.
    if (ui->mainSplitter->sizes().at(0) != 0)
        timeLineSize = defaultTimeLineSize;
    ui->actionTimeline->setChecked(ui->mainSplitter->sizes().at(0) != 0);
}

void MainWindow::on_actionStatusDetails_triggered()
{
    showStatusDetails = !showStatusDetails;
    ui->nextEventLabel->setVisible(showStatusDetails);
    ui->nextModuleLabel->setVisible(showStatusDetails);
    ui->nextTimeLabel->setVisible(showStatusDetails);
    updateStatusDisplay();
}

void MainWindow::on_actionFindInspectObjects_triggered()
{
    // implements Find/inspect objects...
    QVariant variant = static_cast<QAction *>(QObject::sender())->data();
    showFindObjectsDialog(variant.value<cObject *>());
}

// debugNextEvent
void MainWindow::on_actionDebugNextEvent_triggered()
{
    // implements Simulate|Debug next event

    if (isRunning())
        QMessageBox::warning(this, tr("错误"), tr("仿真正在运行 -- 请先停下"),
                QMessageBox::Ok);
    else {
        if (!networkReady())
            return;

        setGuiForRunmode(RUNMODE_STEP);
        getSimulation()->requestTrapOnNextEvent();
        on_actionOneStep_triggered();
        setGuiForRunmode(RUNMODE_NOT_RUNNING);
    }
}

void MainWindow::on_actionDebugOnErrors_triggered(bool checked)
{
    getQtenv()->debugOnErrors = checked;
}

void MainWindow::on_actionDebugNow_triggered()
{
    if (env->ensureDebugger())
        DEBUG_TRAP; // YOU ASKED FOR A DEBUGGER IN THE MENU -- PLACE YOUR BREAKPOINTS AND CONTINUE EXECUTION
}

void MainWindow::on_actionEventlogRecording_triggered()
{
    getQtenv()->setEventlogRecording(!getQtenv()->getEventlogRecording());
}

void MainWindow::reflectRecordEventlog()
{
    ui->actionEventlogRecording->setChecked(getQtenv()->getEventlogRecording());
}

// XXX why is this in MainWindow, and not in Qtenv?
// 老外接手时候也是一脸懵逼，为啥这个函数放在这里？
// 这个函数主要是对图中一些未配置的参数询问一下？
void MainWindow::configureNetwork()
{
    if (env->checkRunning())
        return;

    // get list of network names
    QVector<cModuleType *> networks;
    cRegistrationList *types = componentTypes.getInstance();
    for (int i = 0; i < types->size(); i++) {
        cModuleType *t = dynamic_cast<cModuleType *>(types->get(i));
        if (t && t->isNetwork())
            networks.push_back(t);
    }
    std::string localPackage = getQtenv()->getLocalPackage();
    QStringList networkNames;
    QStringList localNetworkNames;
    for (cModuleType *net : networks) {
        const char *networkName = net->getName();
        const char *networkQName = net->getFullName();
        char result[100];
        strcpy(result, localPackage.c_str());
        strcat(result, ".");
        strcat(result, networkName);
        if (strcmp(result, networkQName) == 0)
            localNetworkNames.push_back(networkName);
        else
            networkNames.push_back(networkQName);
    }

    // lamda表达式实现了一个字符串比较的函数，语法稍微简洁一些而已
    auto lessThan = [](QString arg1, QString arg2) -> bool
                    {
                        return arg1.toLower() < arg2.toLower();
                    };

    std::sort(localNetworkNames.begin(), localNetworkNames.end(), lessThan);
    std::sort(networkNames.begin(), networkNames.end(), lessThan);

    // 拼接到第一个，返回拼接后的
    networkNames = localNetworkNames << networkNames;

    // pop up dialog, with current network as default
    cModuleType *networkType = getSimulation()->getNetworkType();
    const char *netName = networkType ? networkType->getName() : nullptr;
    ComboSelectionDialog comboDialog(netName, localNetworkNames);
    if (comboDialog.exec() == QDialog::Accepted) {
        busy("建立网络中...");
        //TODO
        //inspectorList:addAll 1
        getQtenv()->newNetwork(comboDialog.getSelectedNetName().toStdString().c_str());
        reflectRecordEventlog();
        busy();
    }
}

// newNetwork
void MainWindow::on_actionSetUpUnconfiguredNetwork_triggered()
{
    // implements File|New network...
    configureNetwork();
}

void MainWindow::on_actionVerticalLayout_triggered(bool checked)
{
    if (ui->splitter_2->orientation() == Qt::Horizontal)
        saveSplitter("mainwin-right-splittersizes-horiz", ui->splitter_2);

    if (checked) {  // it just got pressed
        ui->actionHorizontalLayout->setChecked(false);
        ui->splitter_2->setOrientation(Qt::Vertical);
        restoreSplitter("mainwin-right-splittersizes-vert", ui->splitter_2);
    }
    else {  // not letting the user uncheck the action
        ui->actionVerticalLayout->setChecked(true);
    }
}

void MainWindow::on_actionHorizontalLayout_triggered(bool checked)
{
    if (ui->splitter_2->orientation() == Qt::Vertical)
        saveSplitter("mainwin-right-splittersizes-vert", ui->splitter_2);

    if (checked) {  // it just got pressed
        ui->actionVerticalLayout->setChecked(false);
        ui->splitter_2->setOrientation(Qt::Horizontal);
        restoreSplitter("mainwin-right-splittersizes-horiz", ui->splitter_2);
    }
    else {  // not letting the user uncheck the action
        ui->actionHorizontalLayout->setChecked(true);
    }
}

void MainWindow::on_actionFlipWindowLayout_triggered()
{
    switch (ui->splitter_2->orientation()) {
        case Qt::Horizontal: ui->actionVerticalLayout->trigger(); break;
        case Qt::Vertical: ui->actionHorizontalLayout->trigger(); break;
    }
}

void MainWindow::on_actionAbout_OMNeT_Qtenv_triggered()
{
    QDialog *about = new QDialog();
    about->setWindowTitle("关于XNET");
    QVBoxLayout *layout = new QVBoxLayout();
    about->setLayout(layout);

    QFrame *frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *frameLayout = new QVBoxLayout();
    frame->setLayout(frameLayout);

    QLabel *label = new QLabel(aboutText);
    label->setFont(getQtenv()->getBoldFont());
    label->setAlignment(Qt::AlignHCenter);
    frameLayout->addWidget(label);

    layout->addWidget(frame);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), about, SLOT(accept()));
    layout->addWidget(buttonBox);
    about->exec();

    delete layout;
    delete frameLayout;
    delete buttonBox;
    delete about;
}

void MainWindow::showFindObjectsDialog(cObject *obj)
{
    // Set mainwindow to dialog's parent, thus it is closed when mainwindow is closed.
    FindObjectsDialog *dialog = new FindObjectsDialog(obj, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    connect(this, SIGNAL(setNewNetwork()), dialog, SLOT(invalidate()));
    connect(this, SIGNAL(closed()), dialog, SLOT(close()));
}

void MainWindow::on_actionLoadNedFile_triggered()
{
    QString lastNedFile = getQtenv()->getPref("last-nedfile", ".").value<QString>();

    QString fileName = QFileDialog::getOpenFileName(this,
                tr("打开NED 文件"), lastNedFile, tr("NED 文件 (*.ned);;所有文件 (*)"));

    if (!fileName.isNull()) {
        getQtenv()->setPref("last-nedfile", fileName);
        getQtenv()->loadNedFile(fileName.toStdString().c_str());

        getQtenv()->callRefreshInspectors();
    }
}

void MainWindow::on_actionOpenPrimaryIniFile_triggered()
{
    QString fileName = getQtenv()->getIniFileName();
    if (fileName.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("当前配置管理未使用文件输入."),
                QMessageBox::Ok);
        return;
    }

    fileEditor->setFile(fileName);
    fileEditor->show();
}

int MainWindow::inputBox(const QString& title, const QString& prompt, QString& variable)
{
    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog->setLayout(layout);
    dialog->setWindowTitle(title);

    // Add widget
    layout->addWidget(new QLabel(prompt));
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setText(variable);
    layout->addWidget(lineEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    dialog->layout()->addWidget(buttonBox);

    int code = dialog->exec();
    variable = lineEdit->text();

    delete dialog;

    return code;
}

// implements File|Create snapshot
void MainWindow::on_actionCreate_Snapshot_triggered()
{
    if (!networkPresent())
        return;

    QString variable = "";

    if (inputBox("快照", "输入当前仿真快照标签:", variable) == QDialog::Accepted) {
        getQtenv()->createSnapshot(variable.toStdString().c_str());

        QString msg = QString(getQtenv()->getSnapshotFileName()).isEmpty()
                ? "仿真状态已经保存."
                : QString("当前仿真状态已经存入\"") + getQtenv()->getSnapshotFileName() + "\".";

        QMessageBox::information(this, tr("快照已完成"), msg, QMessageBox::Ok);
    }
}

void MainWindow::on_actionConcludeSimulation_triggered()
{
    // check state is not SIM_RUNNING
    if (env->checkRunning())
        return;

    // check state is not SIM_NONET
    if (!networkPresent())
        return;

    // check state is not SIM_FINISHCALLED
    if (getQtenv()->getSimulationState() == Qtenv::SIM_FINISHCALLED) {
        QMessageBox::information(this, tr("错误"), tr("finish() 已经调用过了."), QMessageBox::Ok);
        return;
    }

    // check state is not SIM_ERROR
    if (getQtenv()->getSimulationState() == Qtenv::SIM_ERROR) {
        QMessageBox::StandardButton ans =
            QMessageBox::question(this, tr("警告"),
                    "仿真停止时报错, 调用 finish() 可能产生不可预计异常. 是否继续?",
                    QMessageBox::Yes | QMessageBox::No);

        if (ans == QMessageBox::No)
            return;
    }
    else {
        QMessageBox::StandardButton ans =
            QMessageBox::question(this, tr("问题"),
                    "是否总结仿真，并对所有模块调用 finish()?",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (ans == QMessageBox::No)
            return;
    }

    busy("对所有模块逐个调用finish()中...");
    getQtenv()->finishSimulation();
    busy();
}

// inspectSystemModule
void MainWindow::on_actionNetwork_triggered()
{
    // implements Inspect|Toplevel modules...
    if (!networkPresent())
        return;

    getQtenv()->inspect(getSimulation()->getSystemModule(), INSP_DEFAULT, true);
}

// inspectMessageQueue
void MainWindow::on_actionScheduledEvents_triggered()
{
    // implements Inspect|Message queue...
    getQtenv()->inspect(getSimulation()->getFES(), INSP_DEFAULT, true);
}

// inspectSimulation
void MainWindow::on_actionSimulation_triggered()
{
    // implements Inspect|Simulation...
    getQtenv()->inspect(getSimulation(), INSP_DEFAULT, true);
}

// inspectComponentTypes
void MainWindow::on_actionNedComponentTypes_triggered()
{
    getQtenv()->inspect(componentTypes.getInstance(), INSP_DEFAULT, true);
}

// inspectClasses
void MainWindow::on_actionRegisteredClasses_triggered()
{
    getQtenv()->inspect(classes.getInstance(), INSP_DEFAULT, true);
}

void MainWindow::on_actionClassDescriptors_triggered()
{
    getQtenv()->inspect(classDescriptors.getInstance(), INSP_DEFAULT, true);
}

// inspectFunctions
void MainWindow::on_actionNED_Functions_triggered()
{
    getQtenv()->inspect(nedFunctions.getInstance(), INSP_DEFAULT, true);
}

// inspectEnums
void MainWindow::on_actionRegistered_Enums_triggered()
{
    getQtenv()->inspect(enums.getInstance(), INSP_DEFAULT, true);
}

// inspectConfigEntries
void MainWindow::on_actionSupportedConfigurationOption_triggered()
{
    getQtenv()->inspect(configOptions.getInstance(), INSP_DEFAULT, true);
}

void MainWindow::on_actionResultFilters_triggered()
{
    getQtenv()->inspect(resultFilters.getInstance(), INSP_DEFAULT, true);
}

void MainWindow::on_actionResultRecorders_triggered()
{
    getQtenv()->inspect(resultRecorders.getInstance(), INSP_DEFAULT, true);
}

void MainWindow::on_actionMessagePrinters_triggered()
{
    getQtenv()->inspect(messagePrinters.getInstance(), INSP_DEFAULT, true);
}

// inspectBypointer
void MainWindow::on_actionInspectByPointer_triggered()
{
    // implements Inspect|By pointer...
    QString pointer = "ptr";
    int ok = inputBox("通过指针透视...", "输入指针(非法指针会造成致命错误):", pointer);
    if (ok == QDialog::Accepted)
        getQtenv()->inspect(strToPtr(pointer.toStdString().c_str()), INSP_DEFAULT, true);
}

void MainWindow::on_actionRecordVideo_toggled(bool checked)
{
    auto duc = env->getDisplayUpdateController();
    // have to resize the mainwindow to be a size of a multiple of 4 in both dimensions
    // because many video encoders (like x264) demand it
    if (checked) {
        QString configRun = env->getConfigEx()->getActiveConfigName();
        configRun += "#" + QString::number(env->getConfigEx()->getActiveRunNumber());

        VideoRecordingDialog dialog(this, configRun);

        dialog.exec();

        if (dialog.result() == QDialog::Accepted) {
            setFixedSize(width() / 2 * 2, height() / 2 * 2);
            QString base = "frames/" + configRun + "_";
            duc->setFilenameBase(base.toUtf8());
            duc->startVideoRecording();
        }
        else
            ui->actionRecordVideo->setChecked(false);
    } else {
        setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        duc->stopVideoRecording();
    }
}

void MainWindow::on_actionShowAnimationParams_toggled(bool checked)
{
    auto duc = env->getDisplayUpdateController();
    if (checked)
        duc->showDialog(ui->actionShowAnimationParams);
    else
        duc->hideDialog();
    setFocus();
    QApplication::processEvents();
}

void MainWindow::ShowMessageBox(QString str)
{
    QMessageBox msg(this);//对话框设置父组件
    msg.setWindowTitle("Window Title");//对话框标题
    msg.setText("This is a message dialog!");//对话框提示文本
    msg.setIcon(QMessageBox::Information);//设置图标类型
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox:: Cancel | QMessageBox::YesToAll);//对话框上包含的按钮

    if(msg.exec() == QMessageBox::Ok)//模态调用
    {
     qDebug() << " Ok is clicked!";//数据处理
    }
}
// robin added test1
void MainWindow:: on_actionPos()
{
    // 测试二维显示位置新信息更新
    getQtenv()->setModulePositon();
}

void MainWindow::on_actionDown()
{

    getQtenv()->onForeignCMd((char *)"STOP", 3,  NULL);
}
void MainWindow::on_actionUp()
{

    getQtenv()->onForeignCMd((char *)"START", 3, NULL);
}

}  // namespace qtenv
}  // namespace omnetpp
