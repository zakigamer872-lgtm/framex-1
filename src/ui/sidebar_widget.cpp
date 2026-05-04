#include "sidebar_widget.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QFont>

SidebarWidget::SidebarWidget(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void SidebarWidget::setupUI() {
    setObjectName("sidebar");
    setStyleSheet(
        "#sidebar { background: #07071A; border-right: 1px solid #1E1B3A; }");

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(8, 12, 8, 12);

    // ── Logo ───────────────────────────────────────────────
    auto* logoWidget = new QWidget(this);
    logoWidget->setFixedHeight(52);
    auto* logoLayout = new QHBoxLayout(logoWidget);
    logoLayout->setContentsMargins(10, 0, 10, 0);
    auto* logoIcon = new QLabel("🎬");
    QFont icf; icf.setPointSize(18);
    logoIcon->setFont(icf);
    auto* logoText = new QLabel("FrameX");
    QFont lf; lf.setFamily("Segoe UI"); lf.setPointSize(15); lf.setBold(true);
    logoText->setFont(lf);
    logoText->setStyleSheet("color: #C084FC;");
    logoLayout->addWidget(logoIcon);
    logoLayout->addWidget(logoText);
    logoLayout->addStretch();
    layout->addWidget(logoWidget);

    // Section label helper
    auto addSection = [&](const QString& text) {
        auto* lbl = new QLabel(text);
        lbl->setObjectName("sectionLabel");
        lbl->setContentsMargins(12, 8, 0, 4);
        lbl->setStyleSheet(
            "color: #2D2B55; font-size: 10px; font-weight: bold;"
            " letter-spacing: 1px;");
        layout->addWidget(lbl);
    };

    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->setExclusive(true);

    // ── MAIN ───────────────────────────────────────────────
    addSection("MAIN");
    struct NavItem { QString icon; QString label; int idx; };

    QList<NavItem> mainItems = {
        {"⊞", "Dashboard",    0},
        {"⏺", "Recordings",  1},
        {"📡", "Streaming",   2},
        {"🖼", "Screenshots", 3},
    };
    for (auto& item : mainItems) {
        auto* btn = makeNavButton(item.icon, item.label, item.idx);
        layout->addWidget(btn);
        m_btnGroup->addButton(btn, item.idx);
        connect(btn, &QPushButton::clicked,
                [this, idx = item.idx] { emit pageSelected(idx); });
        if (item.idx == 0) btn->setChecked(true);
    }

    // ── TOOLS ──────────────────────────────────────────────
    addSection("TOOLS");
    QList<NavItem> toolItems = {
        {"📤", "Export",    4},
        {"🕐", "Schedule",  5},
        {"✨", "AI Tools",  6},
        {"🏷",  "Watermark", 7},
    };
    for (auto& item : toolItems) {
        auto* btn = makeNavButton(item.icon, item.label, item.idx);
        layout->addWidget(btn);
        m_btnGroup->addButton(btn, item.idx);
        connect(btn, &QPushButton::clicked,
                [this, idx = item.idx] { emit pageSelected(idx); });
    }

    // ── SETTINGS ───────────────────────────────────────────
    addSection("SETTINGS");
    QList<NavItem> setItems = {
        {"⚙",  "Settings", 8},
        {"⌨",  "Hotkeys",  9},
        {"🔧", "Tools",    10},
    };
    for (auto& item : setItems) {
        auto* btn = makeNavButton(item.icon, item.label, item.idx);
        layout->addWidget(btn);
        m_btnGroup->addButton(btn, item.idx);
        connect(btn, &QPushButton::clicked,
                [this, idx = item.idx] { emit pageSelected(idx); });
    }

    layout->addStretch();

    // ── Recording indicator ────────────────────────────────
    m_recordingIndicator = new QLabel("⏺  REC");
    m_recordingIndicator->setAlignment(Qt::AlignCenter);
    m_recordingIndicator->setFixedHeight(28);
    m_recordingIndicator->setStyleSheet(
        "color:#EF4444; font-weight:bold; font-size:12px; padding:4px;"
        "background:#1A0A0A; border-radius:8px; border:1px solid #7F1D1D;");
    m_recordingIndicator->hide();
    layout->addWidget(m_recordingIndicator);

    // ── Storage bar ────────────────────────────────────────
    auto* storageFrame = new QFrame(this);
    storageFrame->setStyleSheet(
        "QFrame{background:#0F0F20;border:1px solid #1E1B3A;border-radius:10px;}");
    auto* sfLayout = new QVBoxLayout(storageFrame);
    sfLayout->setContentsMargins(10, 10, 10, 10);
    sfLayout->setSpacing(6);

    auto* topRow = new QHBoxLayout();
    auto* storageLbl = new QLabel("💾  Storage");
    storageLbl->setStyleSheet("color:#64748B; font-size:11px;");
    m_storageUsedLabel = new QLabel("-- / -- GB");
    m_storageUsedLabel->setStyleSheet("color:#94A3B8; font-size:11px;");
    topRow->addWidget(storageLbl);
    topRow->addStretch();
    topRow->addWidget(m_storageUsedLabel);

    m_storageBar = new QProgressBar(this);
    m_storageBar->setFixedHeight(4);
    m_storageBar->setRange(0, 100);
    m_storageBar->setValue(45);
    m_storageBar->setTextVisible(false);
    m_storageBar->setStyleSheet(
        "QProgressBar{background:#1A1A2E;border:none;border-radius:2px;}"
        "QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #7C3AED,stop:1 #C084FC);border-radius:2px;}");

    sfLayout->addLayout(topRow);
    sfLayout->addWidget(m_storageBar);
    layout->addWidget(storageFrame);

    // ── Separator ──────────────────────────────────────────
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(
        "QFrame{background:#1E1B3A;max-height:1px;border:none;}");
    layout->addWidget(sep);

    // ── Profile card ───────────────────────────────────────
    auto* profileWidget = new QWidget(this);
    auto* profileLayout = new QHBoxLayout(profileWidget);
    profileLayout->setContentsMargins(8, 6, 8, 4);
    profileLayout->setSpacing(8);

    auto* avatarLabel = new QLabel("Z");
    avatarLabel->setFixedSize(36, 36);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #7C3AED,stop:1 #C084FC);"
        "border-radius:18px;color:white;font-weight:bold;font-size:16px;");

    auto* userCol = new QVBoxLayout();
    userCol->setSpacing(1);
    m_userLabel = new QLabel("FrameX Pro");
    m_userLabel->setStyleSheet(
        "color:#E2E8F0;font-size:12px;font-weight:bold;");
    auto* devLabel = new QLabel("By ZAKI  ·  @zg22x");
    devLabel->setStyleSheet("color:#2D2B55;font-size:10px;");
    userCol->addWidget(m_userLabel);
    userCol->addWidget(devLabel);

    auto* badge = new QLabel("PRO");
    badge->setFixedSize(30, 16);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #7C3AED,stop:1 #C084FC);"
        "border-radius:4px;color:white;font-size:9px;font-weight:bold;");

    profileLayout->addWidget(avatarLabel);
    profileLayout->addLayout(userCol, 1);
    profileLayout->addWidget(badge);
    layout->addWidget(profileWidget);

    // ── Read storage info ──────────────────────────────────
    QStorageInfo si(QStandardPaths::writableLocation(
        QStandardPaths::MoviesLocation));
    if (si.isValid() && si.bytesTotal() > 0) {
        double total = si.bytesTotal()  / 1073741824.0;
        double used  = (si.bytesTotal() - si.bytesFree()) / 1073741824.0;
        m_storageUsedLabel->setText(
            QString("%1 / %2 GB").arg(used, 0, 'f', 1).arg(total, 0, 'f', 0));
        m_storageBar->setValue(
            qRound(used / total * 100));
    }
}

QPushButton* SidebarWidget::makeNavButton(const QString& icon,
                                          const QString& text,
                                          int /*index*/) {
    auto* btn = new QPushButton(this);
    btn->setObjectName("sidebarBtn");
    btn->setText("  " + icon + "  " + text);
    btn->setCheckable(true);
    btn->setFixedHeight(42);
    btn->setCursor(Qt::PointingHandCursor);
    QFont f; f.setFamily("Segoe UI"); f.setPointSize(11);
    btn->setFont(f);
    return btn;
}

void SidebarWidget::setRecordingState(bool recording) {
    m_isRecording = recording;
    m_recordingIndicator->setVisible(recording);
}

void SidebarWidget::updateStorageInfo(qint64 used, qint64 total) {
    if (total > 0) {
        double u = used  / 1073741824.0;
        double t = total / 1073741824.0;
        m_storageUsedLabel->setText(
            QString("%1 / %2 GB").arg(u, 0, 'f', 1).arg(t, 0, 'f', 0));
        m_storageBar->setValue(
            qRound(static_cast<double>(used) / total * 100));
    }
}
