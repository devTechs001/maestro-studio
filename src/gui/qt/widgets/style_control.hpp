// src/gui/qt/widgets/style_control.hpp
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

namespace maestro::gui {

class StyleControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit StyleControlWidget(QWidget* parent = nullptr);

    void setStyleName(const QString& name);
    void setTempo(double bpm);
    void setCurrentVariation(int variation);
    void setPlaying(bool playing);

    double tempo() const;
    int currentVariation() const;
    bool isPlaying() const;

signals:
    void styleStartRequested();
    void styleStopRequested();
    void tempoChanged(double bpm);
    void variationChanged(int variation);
    void introRequested(int number);
    void endingRequested(int number);
    void fillRequested(int variation);
    void breakRequested();

private slots:
    void onVariationButtonClicked();
    void onTempoChanged(int value);

private:
    void setupUI();
    void updateVariationButtons();

    QLabel* styleNameLabel_;
    QLabel* tempoLabel_;
    QComboBox* tempoSpinBox_;
    
    QPushButton* startStopButton_;
    QPushButton* syncButton_;
    
    QPushButton* introButton_;
    QPushButton* endingButton_;
    QPushButton* fillButton_;
    QPushButton* breakButton_;
    
    std::vector<QPushButton*> variationButtons_;
    
    int currentVariation_ = 0;
    double tempo_ = 120.0;
    bool playing_ = false;
};

} // namespace maestro::gui
