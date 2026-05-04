#pragma once
#include <QWidget>
#include <QRect>
#include <QRubberBand>
#include <QLabel>

class RegionSelector : public QWidget {
    Q_OBJECT
public:
    static QRect selectRegion();

    explicit RegionSelector(QWidget* parent = nullptr);

signals:
    void regionSelected(const QRect& region);
    void cancelled();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    QPoint       m_origin;
    QRubberBand* m_rubberBand = nullptr;
    QLabel*      m_infoLabel  = nullptr;
    bool         m_selecting  = false;
    QRect        m_selected;
};
