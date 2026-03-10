// src/gui/qt/widgets/mixer.hpp
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <vector>

namespace maestro::gui {

class ChannelStrip : public QWidget {
    Q_OBJECT

public:
    explicit ChannelStrip(const QString& name, int index, QWidget* parent = nullptr);

    void setName(const QString& name);
    void setVolume(float volume);
    void setPan(float pan);
    void setMute(bool mute);
    void setSolo(bool solo);
    void setPeakLevel(float level);

    float volume() const;
    float pan() const;
    bool isMute() const;
    bool isSolo() const;

signals:
    void volumeChanged(int index, float volume);
    void panChanged(int index, float pan);
    void muteChanged(int index, bool mute);
    void soloChanged(int index, bool solo);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int index_;
    QString name_;
    float volume_ = 1.0f;
    float pan_ = 0.0f;
    bool mute_ = false;
    bool solo_ = false;
    float peakLevel_ = 0.0f;
};

class MixerWidget : public QWidget {
    Q_OBJECT

public:
    explicit MixerWidget(QWidget* parent = nullptr);

    void setChannelCount(int count);
    int channelCount() const;

    ChannelStrip& channel(int index);
    void setMasterVolume(float volume);

signals:
    void channelVolumeChanged(int index, float volume);
    void channelPanChanged(int index, float pan);
    void channelMuteChanged(int index, bool mute);
    void channelSoloChanged(int index, bool solo);

private:
    void setupUI();
    void rebuildChannels();

    QHBoxLayout* layout_;
    std::vector<ChannelStrip*> channels_;
    ChannelStrip* masterChannel_;
    int channelCount_ = 8;
};

} // namespace maestro::gui
