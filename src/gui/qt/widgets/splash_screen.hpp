// src/gui/qt/widgets/splash_screen.hpp
#pragma once

#include <QSplashScreen>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>

namespace maestro::gui {

class SplashScreen : public QSplashScreen {
    Q_OBJECT

public:
    explicit SplashScreen(QWidget* parent = nullptr);
    ~SplashScreen() override;

    void showMessage(const QString& message);
    void setProgress(int value, int maximum);
    void finishWithFade(QWidget* mainWidget);

signals:
    void splashFinished();

private slots:
    void onTimeout();

private:
    void drawContents(QPainter* painter) override;

    QProgressBar* progressBar_;
    QLabel* messageLabel_;
    QLabel* versionLabel_;
    QLabel* statusLabel_;
    QTimer* timer_;
    int progress_ = 0;
    int maximum_ = 100;
    QString currentMessage_;
    bool fading_ = false;
};

} // namespace maestro::gui
