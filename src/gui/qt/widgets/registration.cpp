// src/gui/qt/widgets/registration.cpp
#include "registration.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>

namespace maestro::gui {

RegistrationWidget::RegistrationWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void RegistrationWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    
    // Bank display
    auto* bankGroup = new QGroupBox("Registration Bank");
    auto* bankLayout = new QHBoxLayout(bankGroup);
    
    auto* bankDownBtn = new QPushButton("◀");
    bankDownBtn->setFixedWidth(40);
    connect(bankDownBtn, &QPushButton::clicked,
            this, &RegistrationWidget::onBankDown);
    
    bankLabel_ = new QLabel("Bank 1");
    bankLabel_->setAlignment(Qt::AlignCenter);
    bankLabel_->setStyleSheet("font-size: 14px; font-weight: bold;");
    
    auto* bankUpBtn = new QPushButton("▶");
    bankUpBtn->setFixedWidth(40);
    connect(bankUpBtn, &QPushButton::clicked,
            this, &RegistrationWidget::onBankUp);
    
    bankLayout->addWidget(bankDownBtn);
    bankLayout->addWidget(bankLabel_);
    bankLayout->addWidget(bankUpBtn);
    mainLayout->addWidget(bankGroup);
    
    // Current registration display
    currentDisplay_ = new QLabel("Memory 1");
    currentDisplay_->setAlignment(Qt::AlignCenter);
    currentDisplay_->setStyleSheet("font-size: 12px; background-color: #202020; padding: 8px;");
    mainLayout->addWidget(currentDisplay_);
    
    // Memory buttons
    auto* memoryGroup = new QGroupBox("Memory");
    auto* memoryLayout = new QVBoxLayout(memoryGroup);
    
    // Create 8 memory buttons in two rows
    for (int row = 0; row < 2; ++row) {
        auto* rowLayout = new QHBoxLayout();
        for (int col = 0; col < 4; ++col) {
            int index = row * 4 + col;
            auto* btn = new QPushButton(QString::number(index + 1));
            btn->setFixedSize(50, 40);
            btn->setCheckable(true);
            connect(btn, &QPushButton::clicked, [this, index]() {
                onMemoryClicked(index);
            });
            memoryButtons_.push_back(btn);
            rowLayout->addWidget(btn);
        }
        memoryLayout->addLayout(rowLayout);
    }
    
    mainLayout->addWidget(memoryGroup);
    mainLayout->addStretch();
    
    updateDisplay();
}

void RegistrationWidget::setBank(int bank) {
    currentBank_ = std::clamp(bank, 0, 9);
    updateDisplay();
}

int RegistrationWidget::currentBank() const {
    return currentBank_;
}

void RegistrationWidget::setRegistrationName(int index, const QString& name) {
    if (index >= 0 && index < static_cast<int>(memoryButtons_.size())) {
        memoryButtons_[index]->setText(name.left(4));
    }
}

QString RegistrationWidget::registrationName(int index) const {
    if (index >= 0 && index < static_cast<int>(memoryButtons_.size())) {
        return memoryButtons_[index]->text();
    }
    return QString();
}

void RegistrationWidget::onBankUp() {
    currentBank_ = std::min(currentBank_ + 1, 9);
    updateDisplay();
    emit bankChanged(currentBank_);
}

void RegistrationWidget::onBankDown() {
    currentBank_ = std::max(currentBank_ - 1, 0);
    updateDisplay();
    emit bankChanged(currentBank_);
}

void RegistrationWidget::onMemoryClicked(int index) {
    // Deselect all buttons
    for (auto* btn : memoryButtons_) {
        btn->setChecked(false);
    }
    
    // Select clicked button
    memoryButtons_[index]->setChecked(true);
    currentMemory_ = index;
    
    updateDisplay();
    emit registrationSelected(currentBank_, index);
}

void RegistrationWidget::updateDisplay() {
    bankLabel_->setText(QString("Bank %1").arg(currentBank_ + 1));
    currentDisplay_->setText(QString("Memory %1").arg(currentMemory_ + 1));
}

} // namespace maestro::gui
