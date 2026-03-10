// src/gui/qt/widgets/transport.hpp
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>

namespace maestro::gui {

class TransportWidget : public QWidget {
    Q_OBJECT

public:
    explicit TransportWidget(QWidget* parent = nullptr);

    void setPlaying(bool playing);
    void setRecording(bool recording);
    void setTempo(double bpm);
    void setTimeSignature(int numerator, int denominator);
    void setPosition(int bar, int beat, int tick);
    void setMetronomeEnabled(bool enabled);

    bool isPlaying() const;
    bool isRecording() const;
    double tempo() const;

signals:
    void playClicked();
    void stopClicked();
    void recordClicked();
    void rewindClicked();
    void tempoChanged(double bpm);
    void metronomeToggled(bool enabled);

private slots:
    void onPlayClicked();
    void onStopClicked();
    void onRecordClicked();
    void onRewindClicked();
    void onTempoUp();
    void onTempoDown();

private:
    void setupUI();
    void updateDisplay();

    QPushButton* rewindButton_;
    QPushButton* playButton_;
    QPushButton* stopButton_;
    QPushButton* recordButton_;
    
    QLabel* tempoLabel_;
    QLabel* tempoDisplay_;
    QPushButton* tempoUp_;
    QPushButton* tempoDown_;
    
    QLabel* timeSignatureLabel_;
    QLabel* positionLabel_;
    
    QPushButton* metronomeButton_;
    
    bool playing_ = false;
    bool recording_ = false;
    double tempo_ = 120.0;
    int bar_ = 1;
    int beat_ = 1;
    int tick_ = 0;
    bool metronomeEnabled_ = false;
};

} // namespace maestro::gui
