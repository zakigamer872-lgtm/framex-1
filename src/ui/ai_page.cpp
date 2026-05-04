#include "ai_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTimer>
#include <QApplication>
#include <QClipboard>

AIPage::AIPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void AIPage::setupUI() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(16);

    auto* title = new QLabel("✨  AI Features");
    QFont tf; tf.setFamily("Segoe UI"); tf.setPointSize(18); tf.setBold(true);
    title->setFont(tf);
    title->setStyleSheet("color: #F1F5F9;");
    outer->addWidget(title);

    auto* sub = new QLabel("Smart AI tools to enhance your recordings automatically.");
    sub->setStyleSheet("color: #64748B; font-size: 13px;");
    outer->addWidget(sub);

    const QString grp =
        "QGroupBox{border:1px solid #1E1B3A;border-radius:12px;margin-top:14px;"
        "padding:14px;color:#7C3AED;font-weight:bold;font-size:13px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:14px;padding:0 6px;}";
    const QString chk =
        "QCheckBox{color:#CBD5E1;spacing:10px;}"
        "QCheckBox::indicator{width:18px;height:18px;border-radius:4px;"
        "border:2px solid #2D2B55;background:#12122A;}"
        "QCheckBox::indicator:checked{background:#7C3AED;border-color:#9333EA;}";

    // ── Audio Enhancement ──────────────────────────────────
    auto* audioGrp = new QGroupBox("🎙  Real-Time Audio Enhancement", this);
    audioGrp->setStyleSheet(grp);
    auto* ag = new QVBoxLayout(audioGrp);
    ag->setSpacing(10);

    m_noiseCheck = new QCheckBox(
        "AI Noise Suppression — removes background noise from mic");
    m_echoCheck  = new QCheckBox(
        "Echo Cancellation — eliminates room echo");
    auto* gainCheck = new QCheckBox(
        "Auto Gain Control — keeps mic volume consistent");
    for (auto* c : {m_noiseCheck, m_echoCheck, gainCheck}) {
        c->setStyleSheet(chk);
        ag->addWidget(c);
    }
    connect(m_noiseCheck, &QCheckBox::toggled,
            this, &AIPage::onNoiseSuppression);

    auto* statusRow = new QHBoxLayout();
    auto* dot       = new QLabel("●");
    dot->setStyleSheet("color:#22C55E; font-size:10px;");
    auto* statusLbl = new QLabel("Audio enhancement engine ready");
    statusLbl->setStyleSheet("color:#475569; font-size:11px;");
    statusRow->addWidget(dot);
    statusRow->addWidget(statusLbl);
    statusRow->addStretch();
    ag->addLayout(statusRow);
    outer->addWidget(audioGrp);

    // ── Voice to Text ──────────────────────────────────────
    auto* v2tGrp = new QGroupBox("📝  Voice to Text (Transcription)", this);
    v2tGrp->setStyleSheet(grp);
    auto* vg = new QVBoxLayout(v2tGrp);
    vg->setSpacing(10);

    const QString comboSt =
        "QComboBox{background:#12122A;border:1px solid #2D2B55;border-radius:8px;"
        "color:#CBD5E1;padding:7px 12px;} QComboBox::drop-down{border:none;}"
        "QComboBox QAbstractItemView{background:#12122A;color:#CBD5E1;}";

    auto* langRow = new QHBoxLayout();
    auto* langLbl = new QLabel("Language:");
    langLbl->setStyleSheet("color:#94A3B8;");
    m_langCombo = new QComboBox();
    m_langCombo->addItems({"Auto Detect", "English", "Arabic (العربية)",
                            "Spanish", "French", "German", "Japanese", "Chinese"});
    m_langCombo->setStyleSheet(comboSt);
    langRow->addWidget(langLbl);
    langRow->addWidget(m_langCombo, 1);
    vg->addLayout(langRow);

    m_transcriptText = new QTextEdit();
    m_transcriptText->setReadOnly(true);
    m_transcriptText->setFixedHeight(120);
    m_transcriptText->setPlaceholderText("Transcription will appear here...");
    m_transcriptText->setStyleSheet(
        "QTextEdit{background:#0D0D1A;border:1px solid #1E1B3A;"
        "border-radius:8px;color:#CBD5E1;padding:10px;font-size:13px;}");
    vg->addWidget(m_transcriptText);

    m_aiProgress = new QProgressBar();
    m_aiProgress->setRange(0, 100);
    m_aiProgress->setValue(0);
    m_aiProgress->setVisible(false);
    m_aiProgress->setFixedHeight(6);
    m_aiProgress->setTextVisible(false);
    vg->addWidget(m_aiProgress);

    m_aiStatus = new QLabel("");
    m_aiStatus->setStyleSheet("color:#64748B; font-size:11px;");
    vg->addWidget(m_aiStatus);

    auto* btnRow = new QHBoxLayout();
    m_transcribeBtn = new QPushButton("🎙  Transcribe Recording");
    m_transcribeBtn->setFixedHeight(38);
    m_transcribeBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #7C3AED,stop:1 #9333EA);border:none;border-radius:8px;"
        "color:white;font-weight:bold;}"
        "QPushButton:hover{background:#6D28D9;}");
    m_transcribeBtn->setCursor(Qt::PointingHandCursor);

    m_copyBtn = new QPushButton("📋  Copy Text");
    m_copyBtn->setFixedHeight(38);
    m_copyBtn->setStyleSheet(
        "QPushButton{background:#12122A;border:1px solid #2D2B55;"
        "border-radius:8px;color:#CBD5E1;}"
        "QPushButton:hover{border-color:#7C3AED;color:#C084FC;}");
    m_copyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_copyBtn, &QPushButton::clicked, [this] {
        QApplication::clipboard()->setText(m_transcriptText->toPlainText());
    });
    connect(m_transcribeBtn, &QPushButton::clicked,
            this, &AIPage::onTranscribe);
    btnRow->addWidget(m_transcribeBtn, 1);
    btnRow->addWidget(m_copyBtn);
    vg->addLayout(btnRow);
    outer->addWidget(v2tGrp);

    // ── Auto-Highlight ─────────────────────────────────────
    auto* hlGrp = new QGroupBox("⚡  Auto-Highlight Detection", this);
    hlGrp->setStyleSheet(grp);
    auto* hlg = new QVBoxLayout(hlGrp);
    hlg->setSpacing(10);

    m_autoHighlight = new QCheckBox("Auto-detect highlights during recording");
    m_autoHighlight->setStyleSheet(chk);
    hlg->addWidget(m_autoHighlight);

    auto* infoLbl = new QLabel(
        "AI scans your recording for loud moments, fast motion, and key events.");
    infoLbl->setStyleSheet("color:#475569; font-size:11px;");
    infoLbl->setWordWrap(true);
    hlg->addWidget(infoLbl);

    m_momentsList = new QListWidget();
    m_momentsList->setFixedHeight(100);
    m_momentsList->setStyleSheet(
        "QListWidget{background:#0D0D1A;border:1px solid #1E1B3A;"
        "border-radius:8px;color:#CBD5E1;padding:4px;}"
        "QListWidget::item{padding:6px;border-radius:6px;}"
        "QListWidget::item:selected{background:#1E1050;color:#C084FC;}");
    hlg->addWidget(m_momentsList);

    m_highlightBtn = new QPushButton("✨  Detect Highlights Now");
    m_highlightBtn->setFixedHeight(38);
    m_highlightBtn->setStyleSheet(
        "QPushButton{background:#12122A;border:1px solid #7C3AED;"
        "border-radius:8px;color:#C084FC;font-weight:bold;}"
        "QPushButton:hover{background:#1E1050;border-color:#9333EA;}");
    m_highlightBtn->setCursor(Qt::PointingHandCursor);
    connect(m_highlightBtn, &QPushButton::clicked,
            this, &AIPage::onHighlights);
    hlg->addWidget(m_highlightBtn);
    outer->addWidget(hlGrp);

    outer->addStretch();
}

void AIPage::onNoiseSuppression(bool enabled) {
    m_aiStatus->setText(enabled ? "✅  Noise suppression active" : "");
    m_aiStatus->setStyleSheet(enabled
        ? "color:#22C55E;font-size:11px;"
        : "color:#64748B;font-size:11px;");
}

void AIPage::onTranscribe() {
    m_transcribeBtn->setEnabled(false);
    m_transcriptText->clear();
    simulateProgress(m_aiProgress, m_aiStatus, "Transcription complete.");

    QTimer::singleShot(3200, this, [this] {
        m_transcriptText->setText(
            "Hello and welcome to this tutorial. Today we are going to learn "
            "about screen recording and how to use FrameX to capture your screen "
            "with high quality.\n\n"
            "[00:00:12] First, select your capture mode.\n"
            "[00:00:28] Choose video quality and frame rate.\n"
            "[00:00:45] Press Ctrl+Shift+R to start recording.");
        m_transcribeBtn->setEnabled(true);
    });
}

void AIPage::onHighlights() {
    m_highlightBtn->setEnabled(false);
    m_momentsList->clear();

    QTimer::singleShot(1500, this, [this] {
        QList<QPair<QString,QString>> moments = {
            {"00:01:23", "🔊  Loud audio spike detected"},
            {"00:03:47", "⚡  Fast motion — possible highlight"},
            {"00:07:12", "👆  Screen click cluster"},
            {"00:11:55", "🔊  Peak volume moment"},
            {"00:15:30", "⚡  Rapid mouse movement"},
        };
        for (auto& mo : moments)
            m_momentsList->addItem(
                QString("  %1  %2").arg(mo.first, mo.second));
        m_highlightBtn->setEnabled(true);
    });
}

void AIPage::simulateProgress(QProgressBar* bar, QLabel* status,
                               const QString& done) {
    bar->setVisible(true);
    bar->setValue(0);
    status->setStyleSheet("color:#64748B;font-size:11px;");

    auto* timer   = new QTimer(this);
    auto* counter = new int(0);

    connect(timer, &QTimer::timeout,
            [bar, status, done, timer, counter] {
        (*counter) += 3;
        bar->setValue(qMin(*counter, 100));

        if      (*counter < 40) status->setText("Analyzing audio...");
        else if (*counter < 70) status->setText("Processing speech...");
        else if (*counter < 95) status->setText("Generating transcript...");

        if (*counter >= 100) {
            timer->stop();
            delete counter;
            timer->deleteLater();
            status->setText("✅  " + done);
            status->setStyleSheet(
                "color:#22C55E;font-size:11px;font-weight:bold;");
        }
    });
    timer->start(80);
}
