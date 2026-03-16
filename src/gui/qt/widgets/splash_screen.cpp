// src/gui/qt/widgets/splash_screen.cpp
#include "splash_screen.hpp"
#include <QPainter>
#include <QVBoxLayout>
#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>

namespace maestro::gui {

SplashScreen::SplashScreen(QWidget* parent)
    : QSplashScreen(QPixmap(800, 600), parent ? Qt::Window : Qt::SplashScreen)
{
    setFixedSize(800, 600);
    
    // Create central widget for layout
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(40, 380, 40, 40);
    layout->setSpacing(10);
    
    // Status label
    statusLabel_ = new QLabel("Initializing audio engine...");
    statusLabel_->setStyleSheet("color: #aaaaaa; font-size: 12px;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel_);
    
    // Progress bar
    progressBar_ = new QProgressBar();
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setFormat("%p%");
    progressBar_->setStyleSheet(R"(
        QProgressBar {
            background-color: #2d2d2d;
            border: 1px solid #3d3d3d;
            border-radius: 3px;
            text-align: center;
            color: #e0e0e0;
        }
        QProgressBar::chunk {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0078d4, stop:1 #00bcf2);
            border-radius: 2px;
        }
    )");
    layout->addWidget(progressBar_);
    
    // Version label
    versionLabel_ = new QLabel("Version 1.0.0");
    versionLabel_->setStyleSheet("color: #666666; font-size: 10px;");
    versionLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(versionLabel_);
    
    // Message label
    messageLabel_ = new QLabel();
    messageLabel_->setStyleSheet("color: #0078d4; font-size: 11px; font-weight: bold;");
    messageLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(messageLabel_);
    
    centralWidget->move(0, 0);
    centralWidget->setFixedSize(800, 600);
    
    // Auto-hide timer
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &SplashScreen::onTimeout);
}

SplashScreen::~SplashScreen() = default;

void SplashScreen::showMessage(const QString& message) {
    currentMessage_ = message;
    messageLabel_->setText(message);
    repaint();
}

void SplashScreen::setProgress(int value, int maximum) {
    progress_ = value;
    maximum_ = maximum;
    progressBar_->setMaximum(maximum);
    progressBar_->setValue(value);
    repaint();
}

void SplashScreen::finishWithFade(QWidget* mainWidget) {
    fading_ = true;
    
    auto* opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    
    auto* fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnimation->setDuration(500);
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    
    connect(fadeAnimation, &QPropertyAnimation::finished, [this, mainWidget, fadeAnimation]() {
        fadeAnimation->deleteLater();
        graphicsEffect()->deleteLater();
        QSplashScreen::finish(mainWidget);
        emit splashFinished();
    });
    
    fadeAnimation->start(QPropertyAnimation::DeleteWhenStopped);
}

void SplashScreen::drawContents(QPainter* painter) {
    // Background gradient
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(18, 18, 18));
    gradient.setColorAt(0.5, QColor(30, 30, 30));
    gradient.setColorAt(1, QColor(25, 25, 25));
    painter->fillRect(rect(), gradient);
    
    // Logo area (top half)
    QRect logoRect(0, 80, width(), 200);
    
    // Draw circular background for logo
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 120, 212));
    painter->drawEllipse(QPoint(width() / 2, 160), 80, 80);
    
    // Draw music note
    painter->setPen(QPen(Qt::white, 4));
    painter->setFont(QFont("Arial", 60, QFont::Bold));
    painter->drawText(QRect(0, 100, width(), 140), Qt::AlignCenter, "🎵");
    
    // Application name
    painter->setPen(Qt::white);
    painter->setFont(QFont("Segoe UI", 36, QFont::Bold));
    painter->drawText(QRect(0, 220, width(), 60), Qt::AlignCenter, "MaestroStudio");
    
    // Tagline
    painter->setPen(QColor(150, 150, 150));
    painter->setFont(QFont("Segoe UI", 14));
    painter->drawText(QRect(0, 270, width(), 30), Qt::AlignCenter, 
                      "Enterprise Music Production Suite");
    
    // Features list
    painter->setPen(QColor(120, 120, 120));
    painter->setFont(QFont("Segoe UI", 10));
    QStringList features = {
        "🎹 Multi-Track Recording  ",
        "🎼 MIDI Sequencing        ",
        "🎸 Hardware Integration   ",
        "🎚️ Professional Mixing    ",
        "🎵 Style & Pattern Engine "
    };
    
    int featureY = 320;
    for (const auto& feature : features) {
        painter->drawText(QRect(0, featureY, width(), 25), Qt::AlignCenter, feature);
        featureY += 25;
    }
    
    // Copyright
    painter->setPen(QColor(80, 80, 80));
    painter->setFont(QFont("Segoe UI", 9));
    painter->drawText(QRect(0, height() - 30, width(), 20), Qt::AlignCenter,
                      "© 2024 MaestroStudio. All rights reserved.");
}

void SplashScreen::onTimeout() {
    // Auto-progress simulation
    if (progress_ < maximum_) {
        setProgress(progress_ + 1, maximum_);
    }
}

} // namespace maestro::gui
