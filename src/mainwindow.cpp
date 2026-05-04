#include "mainwindow.h"
#include "ui/sidebar_widget.h"
#include "ui/dashboard_page.h"
#include "ui/recordings_page.h"
#include "ui/screenshots_page.h"
#include "ui/settings_page.h"
#include "ui/hotkeys_page.h"
#include "ui/tools_page.h"
#include "ui/streaming_page.h"
#include "ui/export_page.h"
#include "ui/schedule_page.h"
#include "ui/ai_page.h"
#include "ui/watermark_page.h"
#include "ui/recording_settings_panel.h"
#include "ui/floating_widget.h"
#include "ui/stats_bar.h"
#include "recording/recorder.h"
#include "settings_manager.h"
#include "hotkey_manager.h"
#include "history_manager.h"
#include "theme_manager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QFrame>
#include <QMessageBox>
#include <QScreen>
#include <QShortcut>
#include <QStyle>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("FrameX");
    setMinimumSize(1140, 720);
    resize(1320, 800);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    m_recorder = new Recorder(this);

    setupUI();
    setupTray();
    setupHotkeys();

    connect(m_recorder, &Recorder::recordingStarted, this, [this] {
        m_isRecording = true;
        m_isPaused    = false;
        m_recordingStart = QDateTime::currentDateTime();
        m_recordingTimer->start(1000);
        m_sidebar->setRecordingState(true);
        m_statsBar->setRecording(true);
        if (SettingsManager::instance().app.showFloatingWidget)
            m_floatingWidget->show();
        m_floatingWidget->startTimer();
        m_trayIcon->showMessage("FrameX", "Recording started",
                                QSystemTrayIcon::Information, 2000);
    });

    connect(m_recorder, &Recorder::recordingStopped, this, [this](const QString& path) {
        m_isRecording = false;
        m_isPaused    = false;
        m_recordingTimer->stop();
        m_sidebar->setRecordingState(false);
        m_statsBar->setRecording(false);
        m_floatingWidget->hide();
        m_floatingWidget->stopTimer();
        HistoryManager::instance().addEntry(path, m_recordingStart,
                                            QDateTime::currentDateTime());
        m_recordingsPage->loadRecordings();
        m_trayIcon->showMessage("FrameX", "Recording saved!",
                                QSystemTrayIcon::Information, 3000);
    });

    connect(m_recorder, &Recorder::statsUpdated,
            this, &MainWindow::onStatsUpdate);

    connect(m_recorder, &Recorder::errorOccurred, this, [this](const QString& msg) {
        m_trayIcon->showMessage("FrameX Error", msg,
                                QSystemTrayIcon::Warning, 4000);
    });

    connect(m_floatingWidget, &FloatingWidget::stopRequested,
            this, &MainWindow::onStopRecording);
    connect(m_floatingWidget, &FloatingWidget::pauseRequested,
            this, &MainWindow::onPauseRecording);
    connect(m_floatingWidget, &FloatingWidget::screenshotRequested,
            this, &MainWindow::onTakeScreenshot);
}

MainWindow::~MainWindow() {
    SettingsManager::instance().save();
}

void MainWindow::setupUI() {
    auto* rootWidget = new QWidget(this);
    setCentralWidget(rootWidget);
    auto* rootVBox = new QVBoxLayout(rootWidget);
    rootVBox->setContentsMargins(0, 0, 0, 0);
    rootVBox->setSpacing(0);

    // ── Custom title bar ──────────────────────────────────
    auto* titleBar = new QFrame();
    titleBar->setFixedHeight(44);
    titleBar->setObjectName("titleBar");
    titleBar->setStyleSheet(
        "#titleBar { background:#07071A; border-bottom:1px solid #1E1B3A; }");

    auto* tbLayout = new QHBoxLayout(titleBar);
    tbLayout->setContentsMargins(14, 0, 8, 0);
    tbLayout->setSpacing(6);

    auto* appIcon  = new QLabel("🎬");
    auto* titleLbl = new QLabel("FrameX");
    titleLbl->setStyleSheet(
        "color:#C084FC; font-weight:bold; font-size:14px; margin-left:4px;");
    auto* versionLbl = new QLabel("v3.0.0");
    versionLbl->setStyleSheet(
        "color:#7C3AED; font-size:10px; background:#1E1B3A;"
        "border-radius:8px; padding:1px 8px;");

    tbLayout->addWidget(appIcon);
    tbLayout->addWidget(titleLbl);
    tbLayout->addWidget(versionLbl);
    tbLayout->addStretch();

    auto makeWinBtn = [](const QString& sym,
                         const QString& hover) -> QPushButton* {
        auto* b = new QPushButton(sym);
        b->setFixedSize(30, 30);
        b->setStyleSheet(QString(
            "QPushButton{background:#12122A;border:none;border-radius:8px;"
            "color:#9CA3AF;font-size:11px;}"
            "QPushButton:hover{background:%1;color:white;}").arg(hover));
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };
    auto* minBtn   = makeWinBtn("—", "#374151");
    auto* maxBtn   = makeWinBtn("□", "#1D4ED8");
    auto* closeBtn = makeWinBtn("✕", "#DC2626");
    connect(minBtn,   &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(maxBtn,   &QPushButton::clicked, [this] {
        isMaximized() ? showNormal() : showMaximized();
    });
    connect(closeBtn, &QPushButton::clicked, this, &QMainWindow::close);
    tbLayout->addWidget(minBtn);
    tbLayout->addWidget(maxBtn);
    tbLayout->addWidget(closeBtn);
    rootVBox->addWidget(titleBar);

    // ── Content row (sidebar + pages + right panel) ───────
    auto* contentRow    = new QHBoxLayout();
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(0);

    m_sidebar = new SidebarWidget(this);
    m_sidebar->setFixedWidth(224);
    contentRow->addWidget(m_sidebar);

    m_pageStack = new QStackedWidget();

    m_dashboardPage   = new DashboardPage(m_recorder, this);
    m_recordingsPage  = new RecordingsPage(this);
    m_streamingPage   = new StreamingPage(this);
    m_screenshotsPage = new ScreenshotsPage(this);
    m_exportPage      = new ExportPage(this);
    m_schedulePage    = new SchedulePage(this);
    m_aiPage          = new AIPage(this);
    m_watermarkPage   = new WatermarkPage(this);
    m_settingsPage    = new SettingsPage(this);
    m_hotkeysPage     = new HotkeysPage(this);
    m_toolsPage       = new ToolsPage(this);

    // Page indices must match sidebar
    m_pageStack->addWidget(m_dashboardPage);    // 0
    m_pageStack->addWidget(m_recordingsPage);   // 1
    m_pageStack->addWidget(m_streamingPage);    // 2
    m_pageStack->addWidget(m_screenshotsPage);  // 3
    m_pageStack->addWidget(m_exportPage);       // 4
    m_pageStack->addWidget(m_schedulePage);     // 5
    m_pageStack->addWidget(m_aiPage);           // 6
    m_pageStack->addWidget(m_watermarkPage);    // 7
    m_pageStack->addWidget(m_settingsPage);     // 8
    m_pageStack->addWidget(m_hotkeysPage);      // 9
    m_pageStack->addWidget(m_toolsPage);        // 10

    contentRow->addWidget(m_pageStack, 1);

    m_settingsPanel = new RecordingSettingsPanel(this);
    m_settingsPanel->setFixedWidth(256);
    contentRow->addWidget(m_settingsPanel);

    auto* contentWidget = new QWidget();
    contentWidget->setLayout(contentRow);
    rootVBox->addWidget(contentWidget, 1);

    // ── Stats bar ─────────────────────────────────────────
    m_statsBar = new StatsBar(this);
    rootVBox->addWidget(m_statsBar);

    // Floating widget
    m_floatingWidget = new FloatingWidget();
    m_floatingWidget->hide();

    // Recording timer (1s tick for elapsed display)
    m_recordingTimer = new QTimer(this);
    connect(m_recordingTimer, &QTimer::timeout,
            this, &MainWindow::onRecordingTimer);

    connect(m_sidebar, &SidebarWidget::pageSelected,
            this, &MainWindow::onPageSelected);

    connect(m_dashboardPage, &DashboardPage::startRecordingRequested,
            this, &MainWindow::onStartRecording);
    connect(m_dashboardPage, &DashboardPage::screenshotRequested,
            this, &MainWindow::onTakeScreenshot);

    connect(m_schedulePage, &SchedulePage::scheduleTriggered,
            this, &MainWindow::onStartRecording);

    m_mainLayout = contentRow;
}

void MainWindow::setupTray() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(
        QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    m_trayIcon->setToolTip("FrameX v3.0.0");

    m_trayMenu = new QMenu(this);
    m_trayMenu->setStyleSheet(
        "QMenu{background:#12122A;border:1px solid #2D2B55;"
        "border-radius:8px;padding:4px;}"
        "QMenu::item{padding:8px 20px;color:#CBD5E1;border-radius:6px;}"
        "QMenu::item:selected{background:#1E1050;color:#C084FC;}"
        "QMenu::separator{height:1px;background:#1E1B3A;margin:3px 8px;}");

    m_trayMenu->addAction("🎬  Show FrameX", this,
        [this] { show(); raise(); activateWindow(); });
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("⏺  Start Recording", this,
        &MainWindow::onStartRecording);
    m_trayMenu->addAction("■  Stop Recording",  this,
        &MainWindow::onStopRecording);
    m_trayMenu->addAction("📷  Screenshot",       this,
        &MainWindow::onTakeScreenshot);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("✕  Quit",             qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayActivated);
}

void MainWindow::setupHotkeys() {
    auto& hk = SettingsManager::instance().hotkeys;
    auto makeShortcut = [this](const QKeySequence& seq, auto slot) {
        auto* sc = new QShortcut(seq, this);
        sc->setContext(Qt::ApplicationShortcut);
        connect(sc, &QShortcut::activated, this, slot);
    };
    makeShortcut(hk.startStop,
        [this] { m_isRecording ? onStopRecording() : onStartRecording(); });
    makeShortcut(hk.pauseResume, &MainWindow::onPauseRecording);
    makeShortcut(hk.screenshot,  &MainWindow::onTakeScreenshot);
}

// ── Slots ─────────────────────────────────────────────────

void MainWindow::onPageSelected(int index) {
    m_pageStack->setCurrentIndex(index);
}

void MainWindow::onStartRecording() {
    if (m_isRecording) return;
    m_recorder->setSettings(SettingsManager::instance().recording);
    m_recorder->startRecording();
}

void MainWindow::onStopRecording() {
    if (!m_isRecording) return;
    m_recorder->stopRecording();
}

void MainWindow::onPauseRecording() {
    if (!m_isRecording) return;
    if (!m_isPaused) {
        m_recorder->pauseRecording();
        m_floatingWidget->setPaused(true);
        m_recordingTimer->stop();
    } else {
        m_recorder->resumeRecording();
        m_floatingWidget->setPaused(false);
        m_recordingTimer->start(1000);
    }
    m_isPaused = !m_isPaused;
}

void MainWindow::onTakeScreenshot() {
    QString dir = QStandardPaths::writableLocation(
                      QStandardPaths::PicturesLocation)
                  + "/FrameX Screenshots";
    QDir().mkpath(dir);
    QString path = dir + "/Screenshot_"
                 + QDateTime::currentDateTime()
                       .toString("yyyy-MM-dd_HH-mm-ss")
                 + ".png";
    if (auto* screen = QApplication::primaryScreen()) {
        if (screen->grabWindow(0).save(path, "PNG")) {
            m_trayIcon->showMessage("FrameX", "Screenshot saved!",
                                    QSystemTrayIcon::Information, 2000);
            m_screenshotsPage->loadScreenshots();
        }
    }
}

void MainWindow::onRecordingTimer() {
    qint64 secs =
        m_recordingStart.secsTo(QDateTime::currentDateTime());
    m_floatingWidget->updateTime(secs);
    m_dashboardPage->updateRecordingTime(secs);

    int h = static_cast<int>(secs / 3600);
    int m = static_cast<int>((secs % 3600) / 60);
    int s = static_cast<int>(secs % 60);
    QString t = QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
    m_trayIcon->setToolTip("FrameX — Recording " + t);
}

void MainWindow::onStatsUpdate(double cpu, double gpu, int fps, qint64 disk) {
    qint64 ramMB = 180 + static_cast<qint64>(
        QRandomGenerator::global()->bounded(40u));
    m_statsBar->updateStats(cpu, gpu, fps, disk, ramMB);
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        show(); raise(); activateWindow();
    }
}

// ── Window events ─────────────────────────────────────────

void MainWindow::closeEvent(QCloseEvent* event) {
    if (SettingsManager::instance().app.minimizeToTray && !m_forceQuit) {
        hide();
        event->ignore();
        m_trayIcon->showMessage("FrameX", "Running in system tray",
                                QSystemTrayIcon::Information, 2000);
        return;
    }
    if (m_isRecording) {
        auto reply = QMessageBox::question(this, "FrameX",
            "A recording is in progress. Stop and quit?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) { event->ignore(); return; }
        m_recorder->stopRecording();
    }
    SettingsManager::instance().save();
    qApp->quit();
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
}

void MainWindow::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && e->pos().y() < 44) {
        m_dragging    = true;
        m_dragStartPos = e->globalPosition().toPoint()
                       - frameGeometry().topLeft();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragging && (e->buttons() & Qt::LeftButton))
        move(e->globalPosition().toPoint() - m_dragStartPos);
}

void MainWindow::mouseReleaseEvent(QMouseEvent*) {
    m_dragging = false;
}
