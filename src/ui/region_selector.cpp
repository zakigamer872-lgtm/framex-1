#include "region_selector.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QEventLoop>
#include <QFont>
#include <QRubberBand>

RegionSelector::RegionSelector(QWidget* parent)
    : QWidget(parent,
              Qt::FramelessWindowHint
              | Qt::WindowStaysOnTopHint
              | Qt::Tool)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    if (auto* screen = QApplication::primaryScreen())
        setGeometry(screen->geometry());

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    m_infoLabel = new QLabel(
        "Drag to select capture region  ·  ESC to cancel", this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setFixedHeight(32);
    m_infoLabel->setStyleSheet(
        "background:rgba(124,58,237,0.92);color:white;font-size:13px;"
        "font-weight:bold;border-radius:8px;padding:0 16px;");
    m_infoLabel->adjustSize();
    m_infoLabel->move((width() - m_infoLabel->width()) / 2, 20);
}

// ── Static helper ─────────────────────────────────────────

QRect RegionSelector::selectRegion() {
    QRect   result;
    auto*   sel  = new RegionSelector();
    QEventLoop loop;
    connect(sel, &RegionSelector::regionSelected,
            [&](const QRect& r) { result = r; loop.quit(); });
    connect(sel, &RegionSelector::cancelled, &loop, &QEventLoop::quit);
    sel->show();
    sel->activateWindow();
    loop.exec();
    sel->close();
    return result;
}

// ── Paint ─────────────────────────────────────────────────

void RegionSelector::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 140));

    if (m_selecting && !m_selected.isNull()) {
        QRect nr = m_selected.normalized();

        // Clear selected region
        p.setCompositionMode(QPainter::CompositionMode_Clear);
        p.fillRect(nr, Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        // Border
        p.setPen(QPen(QColor("#7C3AED"), 2, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(nr);

        // Size badge
        QString info = QString("%1 × %2").arg(nr.width()).arg(nr.height());
        QFont f; f.setFamily("Segoe UI"); f.setPointSize(9); f.setBold(true);
        p.setFont(f);
        QRect badge(nr.topLeft() + QPoint(4, 4), QSize(100, 20));
        p.fillRect(badge, QColor(124, 58, 237, 200));
        p.setPen(Qt::white);
        p.drawText(badge, Qt::AlignCenter, info);
    }
}

// ── Mouse events ──────────────────────────────────────────

void RegionSelector::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_origin    = e->pos();
        m_selecting = true;
        m_selected  = QRect(m_origin, QSize());
        m_rubberBand->setGeometry(m_selected);
        m_rubberBand->show();
    }
}

void RegionSelector::mouseMoveEvent(QMouseEvent* e) {
    if (m_selecting) {
        m_selected = QRect(m_origin, e->pos()).normalized();
        m_rubberBand->setGeometry(m_selected);
        update();
    }
}

void RegionSelector::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton && m_selecting) {
        m_selecting = false;
        m_rubberBand->hide();
        QRect r = QRect(m_origin, e->pos()).normalized();
        if (r.width() > 10 && r.height() > 10)
            emit regionSelected(r);
        else
            emit cancelled();
    }
}

void RegionSelector::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape)
        emit cancelled();
}
