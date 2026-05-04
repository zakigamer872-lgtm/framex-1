#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QProgressBar>

class StatsBar : public QWidget {
    Q_OBJECT
public:
    explicit StatsBar(QWidget* parent = nullptr);
    void updateStats(double cpu, double gpu, int fps, qint64 diskBytes, qint64 ramMB);
    void setRecording(bool recording);

private:
    void setupUI();

    QLabel* m_cpuLabel;
    QLabel* m_gpuLabel;
    QLabel* m_fpsLabel;
    QLabel* m_diskLabel;
    QLabel* m_ramLabel;
    QLabel* m_recDotLabel;
    QLabel* m_recTimeLabel;
    bool    m_recording = false;
};
