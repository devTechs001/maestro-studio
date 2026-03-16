// src/gui/qt/widgets/style_control.cpp
#include "style_control.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <algorithm>

namespace maestro::gui {

StyleControlWidget::StyleControlWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void StyleControlWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    
    // Style name
    styleNameLabel_ = new QLabel("No Style Loaded");
    styleNameLabel_->setAlignment(Qt::AlignCenter);
    styleNameLabel_->setStyleSheet("font-weight: bold; font-size: 12px;");
    mainLayout->addWidget(styleNameLabel_);
    
    // Tempo control
    auto* tempoLayout = new QHBoxLayout();
    tempoLabel_ = new QLabel("Tempo:");
    tempoSpinBox_ = new QComboBox();
    tempoSpinBox_->setEditable(true);
    for (int i = 40; i <= 300; i += 5) {
        tempoSpinBox_->addItem(QString::number(i));
    }
    tempoSpinBox_->setCurrentText("120");
    connect(tempoSpinBox_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            [this](const QString& text) {
                tempo_ = text.toDouble();
                emit tempoChanged(tempo_);
            });
    tempoLayout->addWidget(tempoLabel_);
    tempoLayout->addWidget(tempoSpinBox_);
    mainLayout->addLayout(tempoLayout);
    
    // Transport buttons
    auto* transportLayout = new QHBoxLayout();
    startStopButton_ = new QPushButton("▶ Start");
    startStopButton_->setCheckable(true);
    connect(startStopButton_, &QPushButton::toggled, [this](bool checked) {
        if (checked) {
            startStopButton_->setText("■ Stop");
            emit styleStartRequested();
        } else {
            startStopButton_->setText("▶ Start");
            emit styleStopRequested();
        }
        playing_ = checked;
    });
    
    syncButton_ = new QPushButton("Sync");
    syncButton_->setCheckable(true);
    transportLayout->addWidget(startStopButton_);
    transportLayout->addWidget(syncButton_);
    mainLayout->addLayout(transportLayout);
    
    // Variation buttons
    auto* variationGroup = new QGroupBox("Variation");
    auto* variationLayout = new QHBoxLayout(variationGroup);
    
    QStringList variationNames = {"A", "B", "C", "D"};
    for (int i = 0; i < 4; ++i) {
        auto* btn = new QPushButton(variationNames[i]);
        btn->setCheckable(true);
        btn->setFixedWidth(40);
        connect(btn, &QPushButton::clicked, [this, i]() {
            currentVariation_ = i;
            updateVariationButtons();
            emit variationChanged(i);
        });
        variationButtons_.push_back(btn);
        variationLayout->addWidget(btn);
    }
    
    variationButtons_[0]->setChecked(true);
    mainLayout->addWidget(variationGroup);
    
    // Section buttons
    auto* sectionLayout = new QHBoxLayout();
    
    introButton_ = new QPushButton("Intro");
    connect(introButton_, &QPushButton::clicked, [this]() {
        emit introRequested(1);
    });
    
    endingButton_ = new QPushButton("Ending");
    connect(endingButton_, &QPushButton::clicked, [this]() {
        emit endingRequested(1);
    });
    
    fillButton_ = new QPushButton("Fill");
    connect(fillButton_, &QPushButton::clicked, [this]() {
        emit fillRequested(currentVariation_);
    });
    
    breakButton_ = new QPushButton("Break");
    connect(breakButton_, &QPushButton::clicked, [this]() {
        emit breakRequested();
    });
    
    sectionLayout->addWidget(introButton_);
    sectionLayout->addWidget(endingButton_);
    sectionLayout->addWidget(fillButton_);
    sectionLayout->addWidget(breakButton_);
    mainLayout->addLayout(sectionLayout);
    
    mainLayout->addStretch();
}

void StyleControlWidget::setStyleName(const QString& name) {
    styleNameLabel_->setText(name);
}

void StyleControlWidget::setTempo(double bpm) {
    tempo_ = bpm;
    tempoSpinBox_->setCurrentText(QString::number(static_cast<int>(bpm)));
}

void StyleControlWidget::setCurrentVariation(int variation) {
    currentVariation_ = std::clamp(variation, 0, 3);
    updateVariationButtons();
}

void StyleControlWidget::setPlaying(bool playing) {
    playing_ = playing;
    startStopButton_->setChecked(playing);
}

double StyleControlWidget::tempo() const {
    return tempo_;
}

int StyleControlWidget::currentVariation() const {
    return currentVariation_;
}

bool StyleControlWidget::isPlaying() const {
    return playing_;
}

void StyleControlWidget::updateVariationButtons() {
    for (size_t i = 0; i < variationButtons_.size(); ++i) {
        variationButtons_[i]->setChecked(static_cast<int>(i) == currentVariation_);
    }
}

void StyleControlWidget::onVariationButtonClicked() {
    // Handle variation button click
    auto* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        auto it = std::find(variationButtons_.begin(), variationButtons_.end(), button);
        if (it != variationButtons_.end()) {
            int index = static_cast<int>(it - variationButtons_.begin());
            currentVariation_ = index;
            emit variationChanged(index);
        }
    }
}

void StyleControlWidget::onTempoChanged(int value) {
    tempo_ = value;
    emit tempoChanged(value);
}

} // namespace maestro::gui
