#include "export_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

ExportPage::ExportPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void ExportPage::setupUI() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(16);

    auto* title = new QLabel("📤  Export & Presets");
    QFont tf; tf.setFamily("Segoe UI"); tf.setPointSize(18); tf.setBold(true);
    title->setFont(tf);
    title->setStyleSheet("color: #F1F5F9;");
    outer->addWidget(title);

    auto* sub = new QLabel("Choose a platform preset or configure custom export settings.");
    sub->setStyleSheet("color: #64748B; font-size: 13px;");
    outer->addWidget(sub);

    const QString grp =
        "QGroupBox{border:1px solid #1E1B3A;border-radius:12px;margin-top:14px;"
        "padding:14px;color:#7C3AED;font-weight:bold;font-size:13px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:14px;padding:0 6px;}";
    const QString combo =
        "QComboBox{background:#12122A;border:1px solid #2D2B55;border-radius:8px;"
        "color:#CBD5E1;padding:7px 12px;} QComboBox::drop-down{border:none;}"
        "QComboBox QAbstractItemView{background:#12122A;color:#CBD5E1;}";

    // ── Platform preset buttons ────────────────────────────
    auto* presetGrp = new QGroupBox("Platform Presets", this);
    presetGrp->setStyleSheet(grp);
    auto* pg = new QVBoxLayout(presetGrp);
    pg->setSpacing(12);

    struct Preset { QString icon, name, color; };
    QList<Preset> presets = {
        {"▶",  "YouTube",  "#FF0000"},
        {"🎵", "TikTok",   "#010101"},
        {"🐦", "Twitter",  "#1DA1F2"},
        {"📘", "Facebook", "#1877F2"},
        {"🎮", "Twitch",   "#9146FF"},
        {"⚙",  "Custom",   "#7C3AED"},
    };
    auto* presetRow = new QHBoxLayout();
    for (auto& p : presets) {
        auto* btn = new QPushButton(p.icon + "\n" + p.name);
        btn->setFixedSize(90, 70);
        btn->setStyleSheet(QString(
            "QPushButton{background:#12122A;border:2px solid #2D2B55;"
            "border-radius:10px;color:#CBD5E1;font-size:12px;}"
            "QPushButton:hover{border-color:%1;color:white;background:#16162E;}")
            .arg(p.color));
        btn->setCursor(Qt::PointingHandCursor);
        connect(btn, &QPushButton::clicked,
                [this, name = p.name] { applyPreset(name); });
        presetRow->addWidget(btn);
    }
    pg->addLayout(presetRow);

    m_presetCombo = new QComboBox();
    m_presetCombo->addItems({"YouTube 1080p60", "YouTube 4K",
        "TikTok 9:16 1080p", "Twitter 720p", "Instagram Reels", "Custom"});
    m_presetCombo->setStyleSheet(combo);
    connect(m_presetCombo, &QComboBox::currentTextChanged,
            this, &ExportPage::applyPreset);
    pg->addWidget(m_presetCombo);
    outer->addWidget(presetGrp);

    // ── Video settings ────────────────────────────────────
    auto* vidGrp = new QGroupBox("Video Settings", this);
    vidGrp->setStyleSheet(grp);
    auto* vg = new QVBoxLayout(vidGrp);
    vg->setSpacing(10);

    auto addRow = [&](const QString& lbl, QWidget* w) {
        auto* row = new QHBoxLayout();
        auto* lb  = new QLabel(lbl);
        lb->setStyleSheet("color:#94A3B8; min-width:130px;");
        row->addWidget(lb);
        row->addWidget(w, 1);
        vg->addLayout(row);
    };

    m_formatCombo = new QComboBox();
    m_formatCombo->addItems({"MP4 (H.264)", "MP4 (H.265/HEVC)",
                              "MKV", "AVI", "WebM (VP9)", "MOV", "GIF"});
    m_formatCombo->setStyleSheet(combo);

    m_resCombo = new QComboBox();
    m_resCombo->addItems({"3840×2160 (4K)", "2560×1440 (2K)",
                           "1920×1080 (1080p)", "1280×720 (720p)",
                           "854×480 (480p)", "Custom..."});
    m_resCombo->setStyleSheet(combo);

    m_fpsCombo = new QComboBox();
    m_fpsCombo->addItems({"60 fps", "30 fps", "24 fps", "15 fps"});
    m_fpsCombo->setStyleSheet(combo);

    addRow("Format:",      m_formatCombo);
    addRow("Resolution:",  m_resCombo);
    addRow("Frame Rate:",  m_fpsCombo);

    auto* bitrateRow = new QHBoxLayout();
    auto* bitrateLbl = new QLabel("Video Bitrate:");
    bitrateLbl->setStyleSheet("color:#94A3B8; min-width:130px;");
    m_bitrateSlider = new QSlider(Qt::Horizontal);
    m_bitrateSlider->setRange(1000, 50000);
    m_bitrateSlider->setValue(8000);
    m_bitrateLabel = new QLabel("8000 Kbps");
    m_bitrateLabel->setStyleSheet("color:#C084FC; min-width:80px;");
    connect(m_bitrateSlider, &QSlider::valueChanged, [this](int v) {
        m_bitrateLabel->setText(QString("%1 Kbps").arg(v));
    });
    bitrateRow->addWidget(bitrateLbl);
    bitrateRow->addWidget(m_bitrateSlider, 1);
    bitrateRow->addWidget(m_bitrateLabel);
    vg->addLayout(bitrateRow);

    m_audioCheck     = new QCheckBox("Include audio track");
    m_watermarkCheck = new QCheckBox("Apply watermark / overlay");
    for (auto* c : {m_audioCheck, m_watermarkCheck}) {
        c->setChecked(true);
        c->setStyleSheet(
            "QCheckBox{color:#CBD5E1;}"
            "QCheckBox::indicator{width:16px;height:16px;border-radius:4px;"
            "border:2px solid #2D2B55;background:#12122A;}"
            "QCheckBox::indicator:checked{background:#7C3AED;border-color:#9333EA;}");
        vg->addWidget(c);
    }
    outer->addWidget(vidGrp);

    // ── Output path ───────────────────────────────────────
    auto* outGrp = new QGroupBox("Output File", this);
    outGrp->setStyleSheet(grp);
    auto* og = new QHBoxLayout(outGrp);
    m_outputPath = new QLineEdit(
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
        + "/FrameX Export/output.mp4");
    m_outputPath->setStyleSheet(
        "QLineEdit{background:#12122A;border:1px solid #2D2B55;"
        "border-radius:8px;color:#CBD5E1;padding:7px 12px;}"
        "QLineEdit:focus{border-color:#7C3AED;}");
    m_browseBtn = new QPushButton("📁");
    m_browseBtn->setFixedSize(36, 36);
    m_browseBtn->setStyleSheet(
        "QPushButton{background:#12122A;border:1px solid #2D2B55;"
        "border-radius:8px;color:#94A3B8;}"
        "QPushButton:hover{color:#C084FC;}");
    connect(m_browseBtn, &QPushButton::clicked, [this] {
        QString f = QFileDialog::getSaveFileName(
            this, "Save Export", m_outputPath->text(),
            "Video Files (*.mp4 *.mkv *.avi *.webm)");
        if (!f.isEmpty()) m_outputPath->setText(f);
    });
    og->addWidget(m_outputPath, 1);
    og->addWidget(m_browseBtn);
    outer->addWidget(outGrp);

    // ── Progress ──────────────────────────────────────────
    m_progress = new QProgressBar();
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_progress->setVisible(false);
    m_progress->setFixedHeight(8);
    m_progress->setTextVisible(false);
    outer->addWidget(m_progress);

    m_statusLabel = new QLabel("");
    m_statusLabel->setStyleSheet("color:#64748B; font-size:12px;");
    outer->addWidget(m_statusLabel);

    outer->addStretch();

    m_exportBtn = new QPushButton("🚀  Start Export");
    m_exportBtn->setFixedHeight(46);
    m_exportBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #7C3AED,stop:1 #9333EA);border:none;border-radius:10px;"
        "color:white;font-weight:bold;font-size:15px;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #6D28D9,stop:1 #7C3AED);}");
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    connect(m_exportBtn, &QPushButton::clicked, this, &ExportPage::startExport);
    outer->addWidget(m_exportBtn);
}

void ExportPage::applyPreset(const QString& name) {
    if (name.contains("4K")) {
        m_resCombo->setCurrentText("3840×2160 (4K)");
        m_fpsCombo->setCurrentText("60 fps");
        m_bitrateSlider->setValue(35000);
        m_formatCombo->setCurrentText("MP4 (H.264)");
    } else if (name.contains("YouTube") || name.contains("1080")) {
        m_resCombo->setCurrentText("1920×1080 (1080p)");
        m_fpsCombo->setCurrentText("60 fps");
        m_bitrateSlider->setValue(8000);
        m_formatCombo->setCurrentText("MP4 (H.264)");
    } else if (name.contains("TikTok") || name.contains("Reels")) {
        m_resCombo->setCurrentText("1920×1080 (1080p)");
        m_fpsCombo->setCurrentText("30 fps");
        m_bitrateSlider->setValue(6000);
        m_formatCombo->setCurrentText("MP4 (H.264)");
    } else if (name.contains("Twitter")) {
        m_resCombo->setCurrentText("1280×720 (720p)");
        m_fpsCombo->setCurrentText("30 fps");
        m_bitrateSlider->setValue(4000);
    } else if (name.contains("GIF")) {
        m_formatCombo->setCurrentText("GIF");
        m_resCombo->setCurrentText("854×480 (480p)");
        m_fpsCombo->setCurrentText("15 fps");
        m_bitrateSlider->setValue(2000);
    }
}

void ExportPage::startExport() {
    m_exportBtn->setEnabled(false);
    m_progress->setVisible(true);
    m_progress->setValue(0);
    m_statusLabel->setStyleSheet("color:#64748B; font-size:12px;");
    m_statusLabel->setText("Preparing export...");

    // Use a shared counter via QTimer with proper cleanup
    auto* timer   = new QTimer(this);
    auto* counter = new int(0);

    connect(timer, &QTimer::timeout, [this, timer, counter] {
        (*counter) += 2;
        m_progress->setValue(*counter);

        if      (*counter < 30) m_statusLabel->setText("Encoding video track...");
        else if (*counter < 60) m_statusLabel->setText("Encoding audio track...");
        else if (*counter < 85) m_statusLabel->setText("Applying filters...");
        else                    m_statusLabel->setText("Finalizing...");

        if (*counter >= 100) {
            timer->stop();
            delete counter;
            timer->deleteLater();
            m_statusLabel->setText("✅  Export complete!");
            m_statusLabel->setStyleSheet(
                "color:#22C55E; font-size:12px; font-weight:bold;");
            m_exportBtn->setEnabled(true);
        }
    });
    timer->start(60);
}
