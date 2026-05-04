#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QListWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>

class AIPage : public QWidget {
    Q_OBJECT
public:
    explicit AIPage(QWidget* parent = nullptr);

private slots:
    void onTranscribe();
    void onHighlights();
    void onNoiseSuppression(bool enabled);

private:
    void setupUI();
    void simulateProgress(QProgressBar* bar, QLabel* status, const QString& done);

    QCheckBox*   m_noiseCheck;
    QCheckBox*   m_echoCheck;
    QCheckBox*   m_autoHighlight;
    QComboBox*   m_langCombo;
    QTextEdit*   m_transcriptText;
    QPushButton* m_transcribeBtn;
    QPushButton* m_highlightBtn;
    QPushButton* m_copyBtn;
    QProgressBar* m_aiProgress;
    QLabel*       m_aiStatus;
    QListWidget*  m_momentsList;
};
