// src/gui/qt/widgets/voice_select.cpp
#include "voice_select.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QGroupBox>

namespace maestro::gui {

VoiceSelectWidget::VoiceSelectWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void VoiceSelectWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    
    // Part selector
    auto* partLayout = new QHBoxLayout();
    partLayout->addWidget(new QLabel("Part:"));
    partSelector_ = new QComboBox();
    partSelector_->addItems({"Right 1", "Right 2", "Right 3", "Left"});
    connect(partSelector_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoiceSelectWidget::onPartChanged);
    partLayout->addWidget(partSelector_);
    mainLayout->addLayout(partLayout);
    
    // Search box
    searchBox_ = new QLineEdit();
    searchBox_->setPlaceholderText("Search voices...");
    mainLayout->addWidget(searchBox_);
    
    // Category filter
    categoryFilter_ = new QComboBox();
    categoryFilter_->addItem("All Categories");
    categoryFilter_->addItems({"Piano", "E.Piano", "Organ", "Guitar", "Bass", 
                               "Strings", "Choir", "Brass", "Sax", "Woodwind",
                               "Synth", "Pad", "Drums", "Percussion", "FX"});
    mainLayout->addWidget(categoryFilter_);
    
    // Voice list
    voiceList_ = new QListWidget();
    voiceList_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(voiceList_, &QListWidget::itemClicked,
            this, &VoiceSelectWidget::onVoiceSelected);
    mainLayout->addWidget(voiceList_);
    
    // Volume control
    auto* volumeGroup = new QGroupBox("Volume");
    auto* volumeLayout = new QHBoxLayout(volumeGroup);
    volumeSlider_ = new QSlider(Qt::Horizontal);
    volumeSlider_->setRange(0, 127);
    volumeSlider_->setValue(100);
    connect(volumeSlider_, &QSlider::valueChanged,
            this, &VoiceSelectWidget::onVolumeChanged);
    volumeLayout->addWidget(volumeSlider_);
    mainLayout->addWidget(volumeGroup);
    
    // Octave control
    auto* octaveGroup = new QGroupBox("Octave");
    auto* octaveLayout = new QHBoxLayout(octaveGroup);
    octaveSpinBox_ = new QSpinBox();
    octaveSpinBox_->setRange(-2, 2);
    octaveSpinBox_->setValue(0);
    connect(octaveSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &VoiceSelectWidget::onOctaveChanged);
    octaveLayout->addWidget(octaveSpinBox_);
    octaveLayout->addStretch();
    mainLayout->addWidget(octaveGroup);
    
    // Populate with sample voices
    setVoices({
        "Stereo Grand Piano",
        "Bright Piano",
        "E.Piano 1",
        "E.Piano 2",
        "Church Organ",
        "Jazz Organ",
        "Acoustic Guitar",
        "Electric Guitar",
        "Finger Bass",
        "Pick Bass",
        "Violin",
        "Viola",
        "Cello",
        "Choir Aahs",
        "Trumpet",
        "Trombone",
        "Alto Sax",
        "Tenor Sax",
        "Flute",
        "Clarinet",
        "Pad 1 (New Age)",
        "Pad 2 (Warm)",
        "Synth Lead 1",
        "Synth Lead 2"
    });
}

void VoiceSelectWidget::setVoices(const QStringList& voices) {
    voiceList_->clear();
    voiceList_->addItems(voices);
}

void VoiceSelectWidget::setCurrentVoice(int index) {
    voiceList_->setCurrentRow(index);
}

void VoiceSelectWidget::setCurrentPart(int part) {
    currentPart_ = std::clamp(part, 0, 3);
    partSelector_->setCurrentIndex(currentPart_);
}

int VoiceSelectWidget::currentVoiceIndex() const {
    return voiceList_->currentRow();
}

int VoiceSelectWidget::currentPart() const {
    return currentPart_;
}

void VoiceSelectWidget::onVoiceSelected() {
    int index = voiceList_->currentRow();
    if (index >= 0) {
        emit voiceSelected(currentPart_, index);
    }
}

void VoiceSelectWidget::onPartChanged(int index) {
    currentPart_ = index;
    emit partChanged(index);
}

void VoiceSelectWidget::onVolumeChanged(int value) {
    emit volumeChanged(currentPart_, value / 127.0f);
}

void VoiceSelectWidget::onOctaveChanged(int value) {
    emit octaveChanged(currentPart_, value);
}

} // namespace maestro::gui
