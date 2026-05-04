#pragma once
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QLineEdit>
#include <QProgressBar>

class ExportPage : public QWidget {
    Q_OBJECT
public:
    explicit ExportPage(QWidget* parent = nullptr);

private:
    void setupUI();
    void applyPreset(const QString& name);
    void startExport();

    QComboBox*   m_presetCombo;
    QComboBox*   m_formatCombo;
    QComboBox*   m_resCombo;
    QComboBox*   m_fpsCombo;
    QSlider*     m_bitrateSlider;
    QLabel*      m_bitrateLabel;
    QCheckBox*   m_audioCheck;
    QCheckBox*   m_watermarkCheck;
    QLineEdit*   m_outputPath;
    QPushButton* m_browseBtn;
    QPushButton* m_exportBtn;
    QProgressBar* m_progress;
    QLabel*      m_statusLabel;
};
