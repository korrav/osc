#include "oscilloscope.h"
#include <QAction>
#include <QPixmap>
#include <QActionGroup>
#include <QIcon>
#include <QMenuBar>
#include <QStatusBar>
#include <QDebug>
void Oscilloscope::createToolBarControls(ActionsOscilloscope *action)
{
    pToolControls_ = new QToolBar("controls");
    addToolBar(Qt::TopToolBarArea, pToolControls_);
    pToolControls_->addAction(action->pPlay);
    pToolControls_->addAction(action->pSinglePlay);
    pToolControls_->addAction(action->pStop);
    pToolControls_->addSeparator();
    pToolControls_->addAction(action->pZoomDown);
    pToolControls_->addAction(action->pZoomUp);
    pToolControls_->addAction(action->pZoom);
    pToolControls_->addAction(action->pUndo);
    pToolControls_->addAction(action->pRedo);
    pToolControls_->addAction(action->pLocation);
    pToolControls_->addSeparator();
    pToolControls_->addAction(action->pGrid);
    pToolControls_->addSeparator();
    pToolControls_->addAction(action->pBufferVisualization);
    pToolControls_->addAction(action->pTriggers);
    pToolControls_->addAction(action->pSignals);
    pToolControls_->addAction(action->pView);
    pToolControls_->addSeparator();
}

void Oscilloscope::createCentralWidget()
{
    setCentralWidget(screen_);
}

void Oscilloscope::createActions(ScreenWidget* screen)
{
    QActionGroup *gControls = new QActionGroup(this);

    QIcon iconPlay;
    iconPlay.addPixmap(QPixmap(":/play_deactive"),QIcon::Normal, QIcon::Off);
    iconPlay.addPixmap(QPixmap(":/play_active"),QIcon::Normal, QIcon::On);
    action_.pPlay = new QAction(iconPlay, "run", gControls);
    action_.pPlay->setCheckable(true);
    connect(action_.pPlay, SIGNAL(triggered()), clip_, SLOT(setModeScreenRun()));

    QIcon iconSinglePlay;
    iconSinglePlay.addPixmap(QPixmap(":/single_play_deactive"),QIcon::Normal, QIcon::Off);
    iconSinglePlay.addPixmap(QPixmap(":/single_play_active"),QIcon::Normal, QIcon::On);
    action_.pSinglePlay = new QAction(iconSinglePlay, "single run", gControls);
    action_.pSinglePlay->setCheckable(true);
    connect(action_.pSinglePlay, SIGNAL(triggered()), clip_, SLOT(setModeScreenSingleRun()));

    QIcon iconStop;
    iconStop.addPixmap(QPixmap(":/stop_deactive"),QIcon::Normal, QIcon::Off);
    iconStop.addPixmap(QPixmap(":/stop_active"),QIcon::Normal, QIcon::On);
    action_.pStop = new QAction(iconStop, "stop", gControls);
    connect(action_.pStop, SIGNAL(triggered()), clip_, SLOT(setModeScreenStop()));
    connect(clip_, SIGNAL(endSingleRun()), action_.pStop, SLOT(trigger()));
    action_.pStop->setCheckable(true);
    action_.pStop->setChecked(true);

    action_.pZoomDown = new QAction(QIcon(":/zoomDown"),"zoom -", this);
    connect(action_.pZoomDown, SIGNAL(triggered()), screen, SLOT(zoomOut()));
    action_.pZoomUp = new QAction(QIcon(":/zoomUp"),"zoom +", this);
    connect(action_.pZoomUp, SIGNAL(triggered()), screen, SLOT(zoom()));
    action_.pZoom = new QAction(QIcon(":/zoom"),"zoom", this);
    connect(action_.pZoom, SIGNAL(toggled(bool)), screen, SLOT(enableRubberBand(bool)));
    action_.pZoom->setCheckable(true);

    QIcon iconUndo;
    iconUndo.addPixmap(QPixmap(":/undo_disable"),QIcon::Disabled);
    iconUndo.addPixmap(QPixmap(":/undo_enable"),QIcon::Normal);
    action_.pUndo = new QAction(iconUndo,"previous", this);
    action_.pUndo->setEnabled(false);
    connect(action_.pUndo, SIGNAL(triggered()), screen, SLOT(undo()));
    connect(screen, SIGNAL(enableUndo(bool)), action_.pUndo, SLOT(setEnabled(bool)));
    QIcon iconRedo;
    iconRedo.addPixmap(QPixmap(":/redo_disable"),QIcon::Disabled);
    iconRedo.addPixmap(QPixmap(":/redo_enable"),QIcon::Normal);
    action_.pRedo = new QAction(iconRedo,"next", this);
    action_.pRedo->setEnabled(false);
    connect(action_.pRedo, SIGNAL(triggered()), screen, SLOT(redo()));
    connect(screen, SIGNAL(enableRedo(bool)), action_.pRedo, SLOT(setEnabled(bool)));

    QIcon iconGrid;
    iconGrid.addPixmap(QPixmap(":/grid_off"),QIcon::Normal, QIcon::Off);
    iconGrid.addPixmap(QPixmap(":/grid_on"),QIcon::Normal, QIcon::On);
    action_.pGrid = new QAction(iconGrid, "grid", this);
    connect(action_.pGrid, SIGNAL(toggled(bool)), screen, SLOT(enableGrid(bool)));
    action_.pGrid->setCheckable(true);
    action_.pGrid->setChecked(true);
    screen->enableGrid(true);

    action_.pLocation = new QAction(QIcon(":/location"), "location", this);
    connect(action_.pLocation, SIGNAL(triggered()), this, SLOT(callLocationDialog()));

    action_.pBufferVisualization = new QAction(QIcon(":/bufferSizeAndFocus"), "buffer visualization", this);
    connect(action_.pBufferVisualization, SIGNAL(triggered()), this, SLOT(callBufferVisualizationDialog()));

    action_.pTriggers = new QAction(QIcon(":/triggers"), "triggers", this);
    connect(action_.pTriggers, SIGNAL(triggered()), this, SLOT(callSettingTriggerDialog()));

    action_.pSignals = new QAction(QIcon(":/signals"), "signals", this);
    connect(action_.pSignals, SIGNAL(triggered()), this, SLOT(callSettingSignalDialog()));

    action_.pView = new QAction(QIcon(":/view"), "view", this);
    connect(action_.pView, SIGNAL(triggered()), this, SLOT(callSettingViewDialog()));
}

void Oscilloscope::createMenuBar(Oscilloscope::ActionsOscilloscope *action)
{
    QMenu* pFileMenu = new QMenu("&File");

    QMenu* pViewMenu = new QMenu("&View");
    pViewMenu->addAction(action->pZoomDown);
    pViewMenu->addAction(action->pZoomUp);
    pViewMenu->addAction(action->pZoom);
    pViewMenu->addAction(action->pUndo);
    pViewMenu->addAction(action->pRedo);
    pViewMenu->addAction(action->pLocation);
    pViewMenu->addSeparator();
    pViewMenu->addAction(action->pGrid);

    QMenu* pSettingsMenu = new QMenu("&Settings");
    pSettingsMenu->addAction(action->pBufferVisualization);
    pSettingsMenu->addAction(action->pTriggers);
    pSettingsMenu->addAction(action->pSignals);
    pSettingsMenu->addAction(action->pView);

    menuBar()->addMenu(pFileMenu);
    menuBar()->addMenu(pViewMenu);
    menuBar()->addMenu(pSettingsMenu);
}

Oscilloscope::Oscilloscope(QVector<qint32> chs, QString title, QWidget *parent): QMainWindow(parent), chs_(chs) {
    setWindowTitle(title);
    clip_ = new ClipBoard(this);
    screen_ = new ScreenWidget(clip_, this);
    plPosTime_ = new QLabel(this);
    plPosTime_->setFixedWidth(150);
    plPosTime_->setStyleSheet("background: lime");
    statusBar()->addWidget(plPosTime_);
    plPosValue_ = new QLabel(this);
    plPosValue_->setFixedWidth(150);
    plPosValue_->setStyleSheet("background: lime");
    statusBar()->addWidget(plPosValue_);

    pDialogNewLocation_ = new SetupNewLocationWindowVisualization(*screen_, this);
    pDialogChangeBufferVisualization_ = new SettingBufferVisualization(*clip_, this);
    pDialogTrigger_ = new SettingTrigger(chs_,clip_, this);
    pDialogSignal_ = new SettingSignal(chs_, *screen_, this);
    pDialogView_ = new SettingView(*screen_, this);

    createActions(screen_);
    createToolBarControls(&action_);
    createMenuBar(&action_);
    createCentralWidget();
    connect(clip_, SIGNAL(issueDebug(QString)), SLOT(deb_message(const QString&)));
    //    WriteTask* wrTask = new WriteTask(QVariant(qlonglong(0)), QVariant(qlonglong(3)), "test", "/tmp/data/");
    //    clip_->passWriteTask(wrTask);
    connect(screen_, SIGNAL(position(QString,QString)), this, SLOT(setInfoCursorPosition(QString,QString)));
    clip_->start();
}

Oscilloscope::~Oscilloscope()
{
    qDebug() << "Начало уничтожения потока clip_";
    qDebug() << "Был уничтожен поток clip_";
}

void Oscilloscope::pass(const shared_ptr<Unit> &pu)
{
    clip_->pass(pu);
    return;
}

void Oscilloscope::deb_message(const QString &str)
{
    debLab_.setText(str);
    debLab_.show();
    return;
}

void Oscilloscope::setInfoCursorPosition(QString time, QString value)
{
    plPosTime_->setText("time = " + time);
    plPosValue_->setText("value = " + value);
}

void Oscilloscope::callLocationDialog()
{
    callDialog(pDialogNewLocation_);
}

void Oscilloscope::callBufferVisualizationDialog()
{
    callDialog(pDialogChangeBufferVisualization_);
}

void Oscilloscope::callSettingTriggerDialog()
{
    callDialog(pDialogTrigger_);
}

void Oscilloscope::callSettingSignalDialog()
{
    callDialog(pDialogSignal_);
}

void Oscilloscope::callSettingViewDialog()
{
    callDialog(pDialogView_);
}

void Oscilloscope::callDialog(QDialog *pDialog)
{
    pDialog->show();
    pDialog->raise();
    pDialog->activateWindow();
}

