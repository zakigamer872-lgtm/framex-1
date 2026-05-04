#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QColorDialog>

class WatermarkPage : public QWidget {
    Q_OBJECT
public:
    explicit WatermarkPage(QWidget* parent = nullptr);

private:
    void setupUI();
    void updatePreview();
    void pickColor();

    QCheckBox*   m_enableCheck;
    QComboBox*   m_typeCombo;
    QLineEdit*   m_textEdit;
    QPushButton* m_logoBtn;
    QComboBox*   m_positionCombo;
    QSlider*     m_opacitySlider;
    QLabel*      m_opacityLabel;
    QSpinBox*    m_sizeSpin;
    QPushButton* m_colorBtn;
    QLabel*      m_previewLabel;
    QColor       m_textColor = QColor("#FFFFFF");
    QString      m_logoPath;
};
