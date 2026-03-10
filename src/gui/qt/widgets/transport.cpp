// src/gui/qt/widgets/transport.cpp
#include "transport.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

namespace maestro::gui {

TransportWidget::TransportWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void TransportWidget::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 4, 8, 4);
    
    // Transport buttons
    rewindButton_ = new QPushButton("⏮");
    rewindButton_->setFixedSize(40, 32);
    rewindButton_->setToolTip("Rewind");
    connect(rewindButton_, &QPushButton::clicked,
            this, &TransportWidget::onRewindClicked);
    mainLayout->addWidget(rewindButton_);
    
    playButton_ = new QPushButton("▶");
    playButton_->setFixedSize(50, 32);
    playButton_->setCheckable(true);
    playButton_->setToolTip("Play");
    connect(playButton_, &QPushButton::toggled,
            this, &TransportWidget::onPlayClicked);
    mainLayout->addWidget(playButton_);
    
    stopButton_ = new QPushButton("■");
    stopButton_->setFixedSize(40, 32);
    stopButton_->setToolTip("Stop");
    connect(stopButton_, &QPushButton::clicked,
            this, &TransportWidget::onStopClicked);
    mainLayout->addWidget(stopButton_);
    
    recordButton_ = new QPushButton("●");
    recordButton_->setFixedSize(40, 32);
    recordButton_->setCheckable(true);
    recordButton_->setStyleSheet("QPushButton { color: red; }");
    recordButton_->setToolTip("Record");
    connect(recordButton_, &QPushButton::toggled,
            this, &TransportWidget::onRecordClicked);
    mainLayout->addWidget(recordButton_);
    
    mainLayout->addSpacing(20);
    
    // Tempo control
    tempoLabel_ = new QLabel("Tempo:");
    mainLayout->addWidget(tempoLabel_);
    
    tempoDown_ = new QPushButton("-");
    tempoDown_->setFixedSize(24, 24);
    connect(tempoDown_, &QPushButton::clicked,
            this, &TransportWidget::onTempoDown);
    mainLayout->addWidget(tempoDown_);
    
    tempoDisplay_ = new QLabel("120");
    tempoDisplay_->setAlignment(Qt::AlignCenter);
    tempoDisplay_->setMinimumWidth(40);
    tempoDisplay_->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(tempoDisplay_);
    
    tempoUp_ = new QPushButton("+");
    tempoUp_->setFixedSize(24, 24);
    connect(tempoUp_, &QPushButton::clicked,
            this, &TransportWidget::onTempoUp);
    mainLayout->addWidget(tempoUp_);
    
    mainLayout->addSpacing(10);
    
    // Time signature
    timeSignatureLabel_ = new QLabel("4/4");
    timeSignatureLabel_->setAlignment(Qt::AlignCenter);
    timeSignatureLabel_->setMinimumWidth(30);
    mainLayout->addWidget(timeSignatureLabel_);
    
    mainLayout->addSpacing(10);
    
    // Position display
    positionLabel_ = new QLabel("1 | 1 | 000");
    positionLabel_->setAlignment(Qt::AlignCenter);
    positionLabel_->setMinimumWidth(100);
    positionLabel_->setStyleSheet("font-family: monospace; font-size: 12px;");
    mainLayout->addWidget(positionLabel_);
    
    mainLayout->addSpacing(20);
    
    // Metronome
    metronomeButton_ = new QPushButton("🎵");
    metronomeButton_->setFixedSize(32, 32);
    metronomeButton_->setCheckable(true);
    metronomeButton_->setToolTip("Metronome");
    connect(metronomeButton_, &QPushButton::toggled, [this](bool checked) {
        metronomeEnabled_ = checked;
        emit metronomeToggled(checked);
    });
    mainLayout->addWidget(metronomeButton_);
}

void TransportWidget::setPlaying(bool playing) {
    playing_ = playing;
    playButton_->setChecked(playing);
    updateDisplay();
}

void TransportWidget::setRecording(bool recording) {
    recording_ = recording;
    recordButton_->setChecked(recording);
    updateDisplay();
}

void TransportWidget::setTempo(double bpm) {
    tempo_ = bpm;
    tempoDisplay_->setText(QString::number(static_cast<int>(bpm)));
}

void TransportWidget::setTimeSignature(int numerator, int denominator) {
    timeSignatureLabel_->setText(QString("%1/%2").arg(numerator).arg(denominator));
}

void TransportWidget::setPosition(int bar, int beat, int tick) {
    bar_ = bar;
    beat_ = beat;
    tick_ = tick;
    updateDisplay();
}

void TransportWidget::setMetronomeEnabled(bool enabled) {
    metronomeEnabled_ = enabled;
    metronomeButton_->setChecked(enabled);
}

bool TransportWidget::isPlaying() const {
    return playing_;
}

bool TransportWidget::isRecording() const {
    return recording_;
}

double TransportWidget::tempo() const {
    return tempo_;
}

void TransportWidget::updateDisplay() {
    positionLabel_->setText(QString("%1 | %2 | %3")
        .arg(bar_)
        .arg(beat_)
        .arg(tick_, 3, 10, QChar('0')));
}

void TransportWidget::onPlayClicked() {
    emit playClicked();
}

void TransportWidget::onStopClicked() {
    playButton_->setChecked(false);
    recordButton_->setChecked(false);
    playing_ = false;
    recording_ = false;
    emit stopClicked();
}

void TransportWidget::onRecordClicked() {
    if (recordButton_->isChecked()) {
        playButton_->setChecked(true);
        playing_ = true;
    }
    recording_ = recordButton_->isChecked();
    emit recordClicked();
}

void TransportWidget::onRewindClicked() {
    bar_ = 1;
    beat_ = 1;
    tick_ = 0;
    updateDisplay();
    emit rewindClicked();
}

void TransportWidget::onTempoUp() {
    tempo_ = std::min(tempo_ + 1.0, 300.0);
    tempoDisplay_->setText(QString::number(static_cast<int>(tempo_)));
    emit tempoChanged(tempo_);
}

void TransportWidget::onTempoDown() {
    tempo_ = std::max(tempo_ - 1.0, 40.0);
    tempoDisplay_->setText(QString::number(static_cast<int>(tempo_)));
    emit tempoChanged(tempo_);
}

} // namespace maestro::gui
