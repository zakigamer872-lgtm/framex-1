#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimeEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTimer>

class SchedulePage : public QWidget {
    Q_OBJECT
public:
    explicit SchedulePage(QWidget* parent = nullptr);

signals:
    void scheduleTriggered();

private slots:
    void onAddSchedule();
    void onCountdownTick();

private:
    void setupUI();
    void updateCountdown();

    QTimeEdit*   m_timeEdit;
    QComboBox*   m_repeatCombo;
    QSpinBox*    m_durationSpin;
    QCheckBox*   m_autoStopCheck;
    QListWidget* m_scheduleList;
    QPushButton* m_addBtn;
    QPushButton* m_removeBtn;
    QLabel*      m_countdownLabel;
    QLabel*      m_nextLabel;
    QTimer*      m_countdownTimer;
    int          m_secondsLeft = 0;
};
