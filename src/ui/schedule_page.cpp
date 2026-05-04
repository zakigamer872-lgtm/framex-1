#include "schedule_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>

SchedulePage::SchedulePage(QWidget* parent) : QWidget(parent) {
    setupUI();
    m_countdownTimer = new QTimer(this);
    connect(m_countdownTimer, &QTimer::timeout, this, &SchedulePage::onCountdownTick);
}

void SchedulePage::setupUI() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(16);

    auto* title = new QLabel("🕐  Scheduled Recording");
    QFont tf; tf.setFamily("Segoe UI"); tf.setPointSize(18); tf.setBold(true);
    title->setFont(tf);
    title->setStyleSheet("color: #F1F5F9;");
    outer->addWidget(title);

    auto* sub = new QLabel("Set a timer to start or stop recording automatically.");
    sub->setStyleSheet("color: #64748B; font-size: 13px;");
    outer->addWidget(sub);

    const QString grp =
        "QGroupBox{border:1px solid #1E1B3A;border-radius:12px;margin-top:14px;"
        "padding:14px;color:#7C3AED;font-weight:bold;font-size:13px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:14px;padding:0 6px;}";

    // --- Countdown Timer ---
    auto* ctGrp = new QGroupBox("Quick Countdown", this);
    ctGrp->setStyleSheet(grp);
    auto* ctLayout = new QVBoxLayout(ctGrp);
    ctLayout->setSpacing(12);

    // Big countdown display
    m_countdownLabel = new QLabel("00:00:00");
    QFont cdf; cdf.setFamily("Segoe UI"); cdf.setPointSize(42); cdf.setBold(true);
    m_countdownLabel->setFont(cdf);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet("color: #C084FC; letter-spacing: 4px;");
    ctLayout->addWidget(m_countdownLabel);

    m_nextLabel = new QLabel("Set a time below and click Schedule");
    m_nextLabel->setAlignment(Qt::AlignCenter);
    m_nextLabel->setStyleSheet("color: #475569; font-size: 12px;");
    ctLayout->addWidget(m_nextLabel);

    // Quick countdown buttons
    auto* quickRow = new QHBoxLayout();
    for (auto& label : QStringList{"5 min", "10 min", "15 min", "30 min", "1 hour"}) {
        auto* btn = new QPushButton(label);
        btn->setFixedHeight(36);
        btn->setStyleSheet(
            "QPushButton{background:#12122A;border:1px solid #2D2B55;border-radius:8px;color:#CBD5E1;}"
            "QPushButton:hover{background:#1E1050;border-color:#7C3AED;color:#C084FC;}");
        btn->setCursor(Qt::PointingHandCursor);
        int mins = label.split(" ")[0].toInt();
        if (label == "1 hour") mins = 60;
        connect(btn, &QPushButton::clicked, [this, mins]{
            m_secondsLeft = mins * 60;
            updateCountdown();
            m_countdownTimer->start(1000);
            m_nextLabel->setText(QString("Recording starts in %1 minute(s)").arg(mins));
            m_nextLabel->setStyleSheet("color:#22C55E; font-size:12px; font-weight:bold;");
        });
        quickRow->addWidget(btn);
    }
    ctLayout->addLayout(quickRow);
    outer->addWidget(ctGrp);

    // --- Schedule by Time ---
    auto* schedGrp = new QGroupBox("Schedule by Time", this);
    schedGrp->setStyleSheet(grp);
    auto* sg = new QVBoxLayout(schedGrp);
    sg->setSpacing(12);

    auto addRow = [&](const QString& lbl, QWidget* w) {
        auto* row = new QHBoxLayout();
        auto* lb = new QLabel(lbl);
        lb->setStyleSheet("color:#94A3B8; min-width:150px;");
        row->addWidget(lb);
        row->addWidget(w, 1);
        sg->addLayout(row);
    };

    m_timeEdit = new QTimeEdit(QTime::currentTime().addSecs(300));
    m_timeEdit->setDisplayFormat("hh:mm AP");
    m_timeEdit->setStyleSheet(
        "QTimeEdit{background:#12122A;border:1px solid #2D2B55;border-radius:8px;"
        "color:#CBD5E1;padding:7px 12px;} QTimeEdit::up-button,QTimeEdit::down-button{width:20px;}");

    m_repeatCombo = new QComboBox();
    m_repeatCombo->addItems({"Once", "Daily", "Weekdays (Mon–Fri)", "Weekends", "Every Hour"});
    m_repeatCombo->setStyleSheet(
        "QComboBox{background:#12122A;border:1px solid #2D2B55;border-radius:8px;color:#CBD5E1;padding:7px 12px;}"
        "QComboBox::drop-down{border:none;} QComboBox QAbstractItemView{background:#12122A;color:#CBD5E1;}");

    m_durationSpin = new QSpinBox();
    m_durationSpin->setRange(1, 480);
    m_durationSpin->setValue(30);
    m_durationSpin->setSuffix(" minutes");
    m_durationSpin->setStyleSheet(
        "QSpinBox{background:#12122A;border:1px solid #2D2B55;border-radius:8px;color:#CBD5E1;padding:7px 12px;}"
        "QSpinBox::up-button,QSpinBox::down-button{width:20px;}");

    m_autoStopCheck = new QCheckBox("Auto-stop after duration");
    m_autoStopCheck->setChecked(true);
    m_autoStopCheck->setStyleSheet(
        "QCheckBox{color:#CBD5E1;} QCheckBox::indicator{width:16px;height:16px;"
        "border-radius:4px;border:2px solid #2D2B55;background:#12122A;}"
        "QCheckBox::indicator:checked{background:#7C3AED;border-color:#9333EA;}");

    addRow("Start Time:", m_timeEdit);
    addRow("Repeat:", m_repeatCombo);
    addRow("Duration:", m_durationSpin);
    sg->addWidget(m_autoStopCheck);
    outer->addWidget(schedGrp);

    // --- Scheduled List ---
    auto* listGrp = new QGroupBox("Scheduled Jobs", this);
    listGrp->setStyleSheet(grp);
    auto* lg = new QVBoxLayout(listGrp);
    m_scheduleList = new QListWidget();
    m_scheduleList->setFixedHeight(120);
    m_scheduleList->setStyleSheet(
        "QListWidget{background:#0D0D1A;border:1px solid #1E1B3A;border-radius:8px;color:#CBD5E1;padding:4px;}"
        "QListWidget::item{padding:8px;border-radius:6px;}"
        "QListWidget::item:selected{background:#1E1050;color:#C084FC;}");
    lg->addWidget(m_scheduleList);

    auto* btnRow = new QHBoxLayout();
    m_addBtn = new QPushButton("➕  Add Schedule");
    m_addBtn->setFixedHeight(38);
    m_addBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7C3AED,stop:1 #9333EA);"
        "border:none;border-radius:8px;color:white;font-weight:bold;}"
        "QPushButton:hover{background:#6D28D9;}");
    m_addBtn->setCursor(Qt::PointingHandCursor);

    m_removeBtn = new QPushButton("🗑  Remove");
    m_removeBtn->setFixedHeight(38);
    m_removeBtn->setStyleSheet(
        "QPushButton{background:#1A0A0A;border:1px solid #7F1D1D;border-radius:8px;color:#FCA5A5;}"
        "QPushButton:hover{background:#7F1D1D;color:white;}");
    m_removeBtn->setCursor(Qt::PointingHandCursor);

    connect(m_addBtn,    &QPushButton::clicked, this, &SchedulePage::onAddSchedule);
    connect(m_removeBtn, &QPushButton::clicked, [this]{
        delete m_scheduleList->takeItem(m_scheduleList->currentRow());
    });

    btnRow->addWidget(m_addBtn, 1);
    btnRow->addWidget(m_removeBtn);
    lg->addLayout(btnRow);
    outer->addWidget(listGrp);

    outer->addStretch();
}

void SchedulePage::onAddSchedule() {
    QString text = QString("⏰  %1  —  %2  (%3 min)  [%4]")
        .arg(m_timeEdit->time().toString("hh:mm AP"))
        .arg(m_repeatCombo->currentText())
        .arg(m_durationSpin->value())
        .arg(m_autoStopCheck->isChecked() ? "auto-stop" : "manual");
    m_scheduleList->addItem(text);
    m_nextLabel->setText("Next: " + m_timeEdit->time().toString("hh:mm AP"));
    m_nextLabel->setStyleSheet("color:#22C55E; font-size:12px; font-weight:bold;");
}

void SchedulePage::onCountdownTick() {
    if (m_secondsLeft <= 0) {
        m_countdownTimer->stop();
        m_countdownLabel->setText("00:00:00");
        m_countdownLabel->setStyleSheet("color:#EF4444; font-size:42px; font-weight:bold;");
        m_nextLabel->setText("🔴  Recording started!");
        emit scheduleTriggered();
        return;
    }
    --m_secondsLeft;
    updateCountdown();
}

void SchedulePage::updateCountdown() {
    int h = m_secondsLeft / 3600;
    int m = (m_secondsLeft % 3600) / 60;
    int s = m_secondsLeft % 60;
    m_countdownLabel->setText(
        QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));
}
