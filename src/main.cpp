#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QDir>
#include <QStandardPaths>
#include <QScreen>
#include "mainwindow.h"
#include "settings_manager.h"
#include "theme_manager.h"

int main(int argc, char *argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setApplicationName("FrameX");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("ZAKI");
    app.setOrganizationDomain("zg22x.framex");

    // Create data directories
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    for (auto& sub : QStringList{"", "/recordings", "/screenshots", "/temp"})
        QDir().mkpath(dataPath + sub);

    SettingsManager::instance().initialize();
    ThemeManager::instance().applyTheme(app);

    // ── Splash Screen ──────────────────────────────────────
    QPixmap splashPix(660, 400);
    splashPix.fill(Qt::transparent);
    QPainter p(&splashPix);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    QLinearGradient bg(0, 0, 660, 400);
    bg.setColorAt(0, QColor("#06061A"));
    bg.setColorAt(1, QColor("#100828"));
    p.fillRect(0, 0, 660, 400, bg);

    // Border glow
    QPen borderPen(QColor("#7C3AED"), 1);
    p.setPen(borderPen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(1, 1, 658, 398, 18, 18);

    // Inner soft glow
    QRadialGradient glow(330, 190, 180);
    glow.setColorAt(0, QColor(124, 58, 237, 50));
    glow.setColorAt(1, QColor(0, 0, 0, 0));
    p.fillRect(0, 0, 660, 400, glow);

    // Logo
    p.setPen(QColor("#C084FC"));
    QFont lf; lf.setFamily("Segoe UI"); lf.setPointSize(50); lf.setBold(true);
    p.setFont(lf);
    p.drawText(QRect(0, 90, 660, 90), Qt::AlignCenter, "FrameX");

    // Tagline
    p.setPen(QColor("#94A3B8"));
    QFont sf; sf.setFamily("Segoe UI"); sf.setPointSize(13);
    p.setFont(sf);
    p.drawText(QRect(0, 188, 660, 36), Qt::AlignCenter, "Capture everything. Share anything.");

    // Version badge
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#1E1B3A"));
    p.drawRoundedRect(285, 234, 90, 24, 12, 12);
    p.setPen(QColor("#7C3AED"));
    QFont vf; vf.setFamily("Segoe UI"); vf.setPointSize(9); vf.setBold(true);
    p.setFont(vf);
    p.drawText(QRect(285, 234, 90, 24), Qt::AlignCenter, "v3.0.0 NEW");

    // What's new badges
    p.setPen(Qt::NoPen);
    QList<QPair<QColor,QString>> badges = {
        {QColor("#7C3AED"), "✨  AI Tools"},
        {QColor("#1D4ED8"), "📤  Export Presets"},
        {QColor("#065F46"), "🕐  Scheduler"},
        {QColor("#92400E"), "🏷  Watermark"},
    };
    int bx = 70;
    for (auto& b2 : badges) {
        QColor c = b2.first; c.setAlpha(180);
        p.setBrush(c);
        p.drawRoundedRect(bx, 270, 120, 24, 6, 6);
        p.setPen(Qt::white);
        QFont bf; bf.setFamily("Segoe UI"); bf.setPointSize(9);
        p.setFont(bf);
        p.drawText(QRect(bx, 270, 120, 24), Qt::AlignCenter, b2.second);
        p.setPen(Qt::NoPen);
        bx += 130;
    }

    // Loading bar background
    p.setBrush(QColor("#0F0F24"));
    p.drawRoundedRect(100, 320, 460, 6, 3, 3);
    QLinearGradient barGrad(100, 0, 560, 0);
    barGrad.setColorAt(0, QColor("#7C3AED"));
    barGrad.setColorAt(0.5, QColor("#C084FC"));
    barGrad.setColorAt(1, QColor("#7C3AED"));
    p.setBrush(barGrad);
    p.drawRoundedRect(100, 320, 460, 6, 3, 3);

    // Developer credit
    p.setPen(QColor("#2D2B55"));
    QFont df; df.setFamily("Segoe UI"); df.setPointSize(9);
    p.setFont(df);
    p.drawText(QRect(0, 360, 660, 28), Qt::AlignCenter,
               "Developed by ZAKI  ·  Instagram & TikTok: @zg22x");
    p.end();

    QSplashScreen splash(splashPix, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    if (auto* screen = QApplication::primaryScreen())
        splash.move(screen->geometry().center() - QPoint(330, 200));
    splash.show();
    app.processEvents();

    MainWindow* window = new MainWindow();
    QTimer::singleShot(2200, [&splash, window]{
        splash.finish(window);
        window->show();
    });

    return app.exec();
}
