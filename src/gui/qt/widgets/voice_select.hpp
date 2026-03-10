// src/gui/qt/widgets/voice_select.hpp
#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>

namespace maestro::gui {

class VoiceSelectWidget : public QWidget {
    Q_OBJECT

public:
    explicit VoiceSelectWidget(QWidget* parent = nullptr);

    void setVoices(const QStringList& voices);
    void setCurrentVoice(int index);
    void setCurrentPart(int part);
    
    int currentVoiceIndex() const;
    int currentPart() const;

signals:
    void voiceSelected(int part, int voiceIndex);
    void partChanged(int part);
    void volumeChanged(int part, float volume);
    void octaveChanged(int part, int octave);

private slots:
    void onVoiceSelected();
    void onPartChanged(int index);
    void onVolumeChanged(int value);
    void onOctaveChanged(int value);

private:
    void setupUI();

    QComboBox* partSelector_;
    QListWidget* voiceList_;
    QLineEdit* searchBox_;
    QComboBox* categoryFilter_;
    
    QSlider* volumeSlider_;
    QSpinBox* octaveSpinBox_;
    
    int currentPart_ = 0;
};

} // namespace maestro::gui
