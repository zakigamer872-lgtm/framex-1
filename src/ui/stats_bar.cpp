#include "stats_bar.h"
#include <QFrame>

StatsBar::StatsBar(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void StatsBar::setupUI() {
    setFixedHeight(34);
    setObjectName("statsBar");
    setStyleSheet(
        "#statsBar { background: #07071A; border-top: 1px solid #1E1B3A; }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(20);

    m_recDotLabel = new QLabel("⚫");
    m_recDotLabel->setStyleSheet("color:#475569; font-size:10px;");
    layout->addWidget(m_recDotLabel);

    m_recTimeLabel = new QLabel("Not Recording");
    m_recTimeLabel->setStyleSheet("color:#475569; font-size:11px;");
    layout->addWidget(m_recTimeLabel);

    auto* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color:#1E1B3A;");
    sep1->setFixedHeight(18);
    layout->addWidget(sep1);

    m_cpuLabel = new QLabel("CPU: --%");
    m_cpuLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    layout->addWidget(m_cpuLabel);

    m_gpuLabel = new QLabel("GPU: --%");
    m_gpuLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    layout->addWidget(m_gpuLabel);

    m_ramLabel = new QLabel("RAM: -- MB");
    m_ramLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    layout->addWidget(m_ramLabel);

    m_fpsLabel = new QLabel("FPS: --");
    m_fpsLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    layout->addWidget(m_fpsLabel);

    m_diskLabel = new QLabel("File: 0 MB");
    m_diskLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    layout->addWidget(m_diskLabel);

    layout->addStretch();

    auto* brand = new QLabel("FrameX v3.0.0  ·  By ZAKI  @zg22x");
    brand->setStyleSheet("color:#1E1B3A; font-size:10px;");
    layout->addWidget(brand);
}

void StatsBar::updateStats(double cpu, double gpu, int fps, qint64 diskBytes, qint64 ramMB) {
    m_cpuLabel->setText(QString("CPU: %1%").arg(cpu, 0, 'f', 1));
    m_gpuLabel->setText(QString("GPU: %1%").arg(gpu, 0, 'f', 1));
    m_fpsLabel->setText(QString("FPS: %1").arg(fps));
    m_ramLabel->setText(QString("RAM: %1 MB").arg(ramMB));

    double sizeMB = diskBytes / 1048576.0;
    if (sizeMB < 1024)
        m_diskLabel->setText(QString("File: %1 MB").arg(sizeMB, 0, 'f', 1));
    else
        m_diskLabel->setText(QString("File: %1 GB").arg(sizeMB / 1024.0, 0, 'f', 2));

    if (m_recording) {
        QString color = cpu > 80 ? "#EF4444" : cpu > 60 ? "#F59E0B" : "#22C55E";
        m_cpuLabel->setStyleSheet(QString("color:%1; font-size:11px;").arg(color));
    }
}

void StatsBar::setRecording(bool recording) {
    m_recording = recording;
    if (recording) {
        m_recDotLabel->setStyleSheet("color:#EF4444; font-size:10px;");
        m_recDotLabel->setText("🔴");
        m_recTimeLabel->setStyleSheet("color:#EF4444; font-size:11px; font-weight:bold;");
        m_recTimeLabel->setText("Recording...");
    } else {
        m_recDotLabel->setStyleSheet("color:#475569; font-size:10px;");
        m_recDotLabel->setText("⚫");
        m_recTimeLabel->setStyleSheet("color:#475569; font-size:11px;");
        m_recTimeLabel->setText("Not Recording");
        m_cpuLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
        m_gpuLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
        m_fpsLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    }
}
