#include "watermark_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QLabel>
#include <QFileDialog>
#include <QPainter>
#include <QFont>

WatermarkPage::WatermarkPage(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void WatermarkPage::setupUI() {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);
    outer->setSpacing(16);

    auto* title = new QLabel("🏷  Watermark & Overlay");
    QFont tf; tf.setFamily("Segoe UI"); tf.setPointSize(18); tf.setBold(true);
    title->setFont(tf);
    title->setStyleSheet("color: #F1F5F9;");
    outer->addWidget(title);

    auto* sub = new QLabel("Add text or logo overlays to your recordings.");
    sub->setStyleSheet("color: #64748B; font-size: 13px;");
    outer->addWidget(sub);

    const QString grp =
        "QGroupBox{border:1px solid #1E1B3A;border-radius:12px;margin-top:14px;"
        "padding:14px;color:#7C3AED;font-weight:bold;font-size:13px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:14px;padding:0 6px;}";
    const QString inputStyle =
        "QLineEdit{background:#12122A;border:1px solid #2D2B55;border-radius:8px;"
        "color:#CBD5E1;padding:7px 12px;} QLineEdit:focus{border-color:#7C3AED;}";
    const QString comboStyle =
        "QComboBox{background:#12122A;border:1px solid #2D2B55;border-radius:8px;"
        "color:#CBD5E1;padding:7px 12px;} QComboBox::drop-down{border:none;}"
        "QComboBox QAbstractItemView{background:#12122A;color:#CBD5E1;}";

    // --- Enable ---
    auto* ctrlGrp = new QGroupBox("Watermark Settings", this);
    ctrlGrp->setStyleSheet(grp);
    auto* cg = new QVBoxLayout(ctrlGrp);
    cg->setSpacing(12);

    m_enableCheck = new QCheckBox("Enable watermark on recordings");
    m_enableCheck->setChecked(false);
    m_enableCheck->setStyleSheet(
        "QCheckBox{color:#E2E8F0;font-size:13px;spacing:10px;}"
        "QCheckBox::indicator{width:18px;height:18px;border-radius:4px;"
        "border:2px solid #2D2B55;background:#12122A;}"
        "QCheckBox::indicator:checked{background:#7C3AED;border-color:#9333EA;}");
    cg->addWidget(m_enableCheck);

    auto addRow = [&](const QString& lbl, QWidget* w) {
        auto* row = new QHBoxLayout();
        auto* lb = new QLabel(lbl);
        lb->setStyleSheet("color:#94A3B8; min-width:130px;");
        row->addWidget(lb);
        row->addWidget(w, 1);
        cg->addLayout(row);
    };

    m_typeCombo = new QComboBox();
    m_typeCombo->addItems({"Text", "Logo / Image", "Text + Logo"});
    m_typeCombo->setStyleSheet(comboStyle);
    addRow("Type:", m_typeCombo);

    m_textEdit = new QLineEdit("FrameX | @zg22x");
    m_textEdit->setStyleSheet(inputStyle);
    connect(m_textEdit, &QLineEdit::textChanged, this, &WatermarkPage::updatePreview);
    addRow("Text:", m_textEdit);

    auto* logoRow = new QHBoxLayout();
    auto* logoLbl = new QLabel("Logo File:");
    logoLbl->setStyleSheet("color:#94A3B8; min-width:130px;");
    auto* logoPath = new QLineEdit("(no logo selected)");
    logoPath->setReadOnly(true);
    logoPath->setStyleSheet(inputStyle);
    m_logoBtn = new QPushButton("📁");
    m_logoBtn->setFixedSize(36, 36);
    m_logoBtn->setStyleSheet("QPushButton{background:#12122A;border:1px solid #2D2B55;"
                             "border-radius:8px;color:#94A3B8;} QPushButton:hover{color:#C084FC;}");
    connect(m_logoBtn, &QPushButton::clicked, [this, logoPath]{
        m_logoPath = QFileDialog::getOpenFileName(this, "Select Logo",
                     "", "Images (*.png *.jpg *.svg)");
        if (!m_logoPath.isEmpty()) {
            logoPath->setText(m_logoPath.split("/").last());
            updatePreview();
        }
    });
    logoRow->addWidget(logoLbl);
    logoRow->addWidget(logoPath, 1);
    logoRow->addWidget(m_logoBtn);
    cg->addLayout(logoRow);

    m_positionCombo = new QComboBox();
    m_positionCombo->addItems({"Bottom Right", "Bottom Left", "Top Right", "Top Left", "Center"});
    m_positionCombo->setStyleSheet(comboStyle);
    connect(m_positionCombo, &QComboBox::currentTextChanged,
            this, &WatermarkPage::updatePreview);
    addRow("Position:", m_positionCombo);

    auto* opacRow = new QHBoxLayout();
    auto* opacLbl = new QLabel("Opacity:");
    opacLbl->setStyleSheet("color:#94A3B8; min-width:130px;");
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(10, 100);
    m_opacitySlider->setValue(80);
    m_opacityLabel  = new QLabel("80%");
    m_opacityLabel->setStyleSheet("color:#C084FC; min-width:40px;");
    connect(m_opacitySlider, &QSlider::valueChanged, [this](int v){
        m_opacityLabel->setText(QString("%1%").arg(v));
        updatePreview();
    });
    opacRow->addWidget(opacLbl);
    opacRow->addWidget(m_opacitySlider, 1);
    opacRow->addWidget(m_opacityLabel);
    cg->addLayout(opacRow);

    auto* sizeRow = new QHBoxLayout();
    auto* sizeLbl = new QLabel("Font Size:");
    sizeLbl->setStyleSheet("color:#94A3B8; min-width:130px;");
    m_sizeSpin = new QSpinBox();
    m_sizeSpin->setRange(8, 72);
    m_sizeSpin->setValue(22);
    m_sizeSpin->setStyleSheet(
        "QSpinBox{background:#12122A;border:1px solid #2D2B55;border-radius:8px;"
        "color:#CBD5E1;padding:6px 12px;} QSpinBox::up-button,QSpinBox::down-button{width:20px;}");
    connect(m_sizeSpin, &QSpinBox::valueChanged, this, &WatermarkPage::updatePreview);
    sizeRow->addWidget(sizeLbl);
    sizeRow->addWidget(m_sizeSpin, 1);
    cg->addLayout(sizeRow);

    auto* colorRow = new QHBoxLayout();
    auto* colorLbl = new QLabel("Text Color:");
    colorLbl->setStyleSheet("color:#94A3B8; min-width:130px;");
    m_colorBtn = new QPushButton("⬛  White");
    m_colorBtn->setFixedHeight(36);
    m_colorBtn->setStyleSheet("QPushButton{background:#12122A;border:1px solid #2D2B55;"
                              "border-radius:8px;color:#CBD5E1;}"
                              "QPushButton:hover{border-color:#7C3AED;}");
    m_colorBtn->setCursor(Qt::PointingHandCursor);
    connect(m_colorBtn, &QPushButton::clicked, this, &WatermarkPage::pickColor);
    colorRow->addWidget(colorLbl);
    colorRow->addWidget(m_colorBtn, 1);
    cg->addLayout(colorRow);
    outer->addWidget(ctrlGrp);

    // --- Preview ---
    auto* prevGrp = new QGroupBox("Preview", this);
    prevGrp->setStyleSheet(grp);
    auto* pg = new QVBoxLayout(prevGrp);
    m_previewLabel = new QLabel();
    m_previewLabel->setFixedHeight(180);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("background:#09091A;border-radius:8px;");
    pg->addWidget(m_previewLabel);
    outer->addWidget(prevGrp);

    outer->addStretch();
    updatePreview();
}

void WatermarkPage::updatePreview() {
    QPixmap px(400, 180);
    px.fill(QColor("#09091A"));
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    // Fake screen content
    p.setPen(QColor("#1E1B3A"));
    for (int y = 20; y < 180; y += 20)
        p.drawLine(0, y, 400, y);
    p.setPen(QColor("#2D2B55"));
    p.drawText(QRect(0, 70, 400, 40), Qt::AlignCenter, "[ Screen Content ]");

    // Watermark text
    int opacity = m_opacitySlider ? m_opacitySlider->value() : 80;
    QColor wc = m_textColor;
    wc.setAlpha(qRound(opacity * 2.55));
    p.setPen(wc);

    QFont wf; wf.setFamily("Segoe UI"); wf.setPointSize(m_sizeSpin ? m_sizeSpin->value() / 2 : 11);
    wf.setBold(true);
    p.setFont(wf);

    QString pos  = m_positionCombo ? m_positionCombo->currentText() : "Bottom Right";
    QString text = m_textEdit ? m_textEdit->text() : "FrameX";
    Qt::AlignmentFlag hAlign = Qt::AlignRight, vAlign = Qt::AlignBottom;
    if (pos.contains("Left"))   hAlign = Qt::AlignLeft;
    if (pos.contains("Top"))    vAlign = Qt::AlignTop;
    if (pos == "Center")        hAlign = Qt::AlignHCenter, vAlign = Qt::AlignVCenter;

    p.drawText(px.rect().adjusted(12, 12, -12, -12),
               Qt::Alignment(hAlign | vAlign), text);
    p.end();
    m_previewLabel->setPixmap(px);
}

void WatermarkPage::pickColor() {
    QColor c = QColorDialog::getColor(m_textColor, this, "Pick Text Color");
    if (c.isValid()) {
        m_textColor = c;
        m_colorBtn->setStyleSheet(QString(
            "QPushButton{background:%1;border:1px solid #2D2B55;border-radius:8px;color:%2;}"
            "QPushButton:hover{border-color:#7C3AED;}").arg(c.name()).arg(
            c.lightness() > 128 ? "#000000" : "#FFFFFF"));
        m_colorBtn->setText(c.name().toUpper());
        updatePreview();
    }
}
