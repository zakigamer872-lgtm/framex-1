#pragma once
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QStackedWidget>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QStyle>
#include <QMenu>

class SidebarWidget;
class DashboardPage;
class RecordingsPage;
class ScreenshotsPage;
class SettingsPage;
class HotkeysPage;
class ToolsPage;
class StreamingPage;
class ExportPage;
class SchedulePage;
class AIPage;
class WatermarkPage;
class RecordingSettingsPanel;
class FloatingWidget;
class StatsBar;
class Recorder;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void forceQuit() { m_forceQuit = true; close(); }

protected:
    void closeEvent(QCloseEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private slots:
    void onPageSelected(int index);
    void onStartRecording();
    void onStopRecording();
    void onPauseRecording();
    void onTakeScreenshot();
    void onRecordingTimer();
    void onStatsUpdate(double cpu, double gpu, int fps, qint64 disk);
    void onTrayActivated(QSystemTrayIcon::ActivationReason);

private:
    void setupUI();
    void setupTray();
    void setupHotkeys();

    QWidget*       m_centralWidget  = nullptr;
    QHBoxLayout*   m_mainLayout     = nullptr;
    SidebarWidget* m_sidebar        = nullptr;
    QStackedWidget* m_pageStack     = nullptr;
    RecordingSettingsPanel* m_settingsPanel = nullptr;
    StatsBar*      m_statsBar       = nullptr;

    DashboardPage*    m_dashboardPage  = nullptr;
    RecordingsPage*   m_recordingsPage = nullptr;
    ScreenshotsPage*  m_screenshotsPage = nullptr;
    SettingsPage*     m_settingsPage   = nullptr;
    HotkeysPage*      m_hotkeysPage    = nullptr;
    ToolsPage*        m_toolsPage      = nullptr;
    StreamingPage*    m_streamingPage  = nullptr;
    ExportPage*       m_exportPage     = nullptr;
    SchedulePage*     m_schedulePage   = nullptr;
    AIPage*           m_aiPage         = nullptr;
    WatermarkPage*    m_watermarkPage  = nullptr;

    QSystemTrayIcon* m_trayIcon  = nullptr;
    QMenu*           m_trayMenu  = nullptr;

    Recorder*       m_recorder        = nullptr;
    FloatingWidget* m_floatingWidget  = nullptr;
    QTimer*         m_recordingTimer  = nullptr;
    QDateTime       m_recordingStart;
    bool m_isRecording = false;
    bool m_isPaused    = false;
    bool m_forceQuit   = false;

    bool   m_dragging     = false;
    QPoint m_dragStartPos;
};
