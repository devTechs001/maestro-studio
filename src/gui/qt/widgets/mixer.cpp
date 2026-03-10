// src/gui/qt/widgets/mixer.cpp
#include "mixer.hpp"
#include <QPainter>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyleOptionSlider>

namespace maestro::gui {

ChannelStrip::ChannelStrip(const QString& name, int index, QWidget* parent)
    : QWidget(parent)
    , index_(index)
    , name_(name)
{
    setFixedSize(80, 300);
    setAutoFillBackground(true);
    
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(40, 40, 40));
    setPalette(pal);
}

void ChannelStrip::setName(const QString& name) {
    name_ = name;
    update();
}

void ChannelStrip::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
    update();
}

void ChannelStrip::setPan(float pan) {
    pan_ = std::clamp(pan, -1.0f, 1.0f);
    update();
}

void ChannelStrip::setMute(bool mute) {
    mute_ = mute;
    update();
}

void ChannelStrip::setSolo(bool solo) {
    solo_ = solo;
    update();
}

void ChannelStrip::setPeakLevel(float level) {
    peakLevel_ = std::clamp(level, 0.0f, 1.0f);
    update();
}

float ChannelStrip::volume() const {
    return volume_;
}

float ChannelStrip::pan() const {
    return pan_;
}

bool ChannelStrip::isMute() const {
    return mute_;
}

bool ChannelStrip::isSolo() const {
    return solo_;
}

void ChannelStrip::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    painter.fillRect(rect(), QColor(45, 45, 45));
    
    // Channel name
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 8));
    painter.drawText(rect().adjusted(5, 5, -5, 25), Qt::AlignCenter, name_);
    
    // Volume fader track
    QRect faderRect = rect().adjusted(30, 35, -30, -60);
    painter.fillRect(faderRect, QColor(20, 20, 20));
    
    // Volume fader handle
    int handleY = faderRect.bottom() - static_cast<int>(volume_ * faderRect.height());
    QRect handleRect(faderRect.left() - 2, handleY - 10, faderRect.width() + 4, 20);
    painter.fillRect(handleRect, mute_ ? QColor(100, 100, 100) : QColor(100, 149, 237));
    painter.setPen(QColor(60, 80, 120));
    painter.drawRect(handleRect);
    
    // Pan knob (simplified as horizontal bar)
    QRect panRect = rect().adjusted(15, faderRect.bottom() + 10, -15, faderRect.bottom() + 20);
    painter.fillRect(panRect, QColor(20, 20, 20));
    int panX = panRect.left() + static_cast<int>((pan_ + 1.0f) / 2.0f * panRect.width());
    painter.fillRect(panX - 3, panRect.top(), 6, panRect.height(), QColor(100, 149, 237));
    
    // Mute/Solo buttons
    QRect muteRect(10, faderRect.bottom() + 35, 25, 20);
    QRect soloRect(45, faderRect.bottom() + 35, 25, 20);
    
    painter.fillRect(muteRect, mute_ ? QColor(200, 50, 50) : QColor(60, 60, 60));
    painter.setPen(mute_ ? Qt::white : Qt::lightGray);
    painter.drawText(muteRect, Qt::AlignCenter, "M");
    
    painter.fillRect(soloRect, solo_ ? QColor(200, 200, 50) : QColor(60, 60, 60));
    painter.setPen(solo_ ? Qt::black : Qt::lightGray);
    painter.drawText(soloRect, Qt::AlignCenter, "S");
    
    // Peak meter
    QRect meterRect = rect().adjusted(5, 35, 12, faderRect.bottom());
    painter.fillRect(meterRect, QColor(20, 20, 20));
    
    int meterHeight = static_cast<int>(peakLevel_ * meterRect.height());
    QRect levelRect = meterRect.adjusted(0, meterRect.height() - meterHeight, 0, 0);
    
    // Color gradient for meter
    QGradient gradient = QLinearGradient(0, meterRect.bottom(), 0, meterRect.top());
    gradient.setColorAt(0, Qt::green);
    gradient.setColorAt(0.7, Qt::yellow);
    gradient.setColorAt(0.9, Qt::red);
    
    painter.fillRect(levelRect, gradient);
}

// MixerWidget implementation

MixerWidget::MixerWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void MixerWidget::setupUI() {
    layout_ = new QHBoxLayout(this);
    layout_->setSpacing(2);
    layout_->setContentsMargins(4, 4, 4, 4);
    
    rebuildChannels();
    
    // Add master channel
    masterChannel_ = new ChannelStrip("Master", -1, this);
    masterChannel_->setFixedWidth(100);
    layout_->addWidget(masterChannel_);
}

void MixerWidget::rebuildChannels() {
    // Clear existing channels
    for (auto* channel : channels_) {
        layout_->removeWidget(channel);
        delete channel;
    }
    channels_.clear();
    
    // Create new channels
    for (int i = 0; i < channelCount_; ++i) {
        auto* channel = new ChannelStrip(QString("Ch %1").arg(i + 1), i, this);
        channels_.push_back(channel);
        layout_->addWidget(channel);
        
        connect(channel, &ChannelStrip::volumeChanged,
                this, &MixerWidget::channelVolumeChanged);
        connect(channel, &ChannelStrip::panChanged,
                this, &MixerWidget::channelPanChanged);
        connect(channel, &ChannelStrip::muteChanged,
                this, &MixerWidget::channelMuteChanged);
        connect(channel, &ChannelStrip::soloChanged,
                this, &MixerWidget::channelSoloChanged);
    }
}

void MixerWidget::setChannelCount(int count) {
    channelCount_ = count;
    rebuildChannels();
}

int MixerWidget::channelCount() const {
    return channelCount_;
}

ChannelStrip& MixerWidget::channel(int index) {
    return *channels_[index];
}

void MixerWidget::setMasterVolume(float volume) {
    masterChannel_->setVolume(volume);
}

} // namespace maestro::gui
