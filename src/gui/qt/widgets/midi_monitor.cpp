// src/gui/qt/widgets/midi_monitor.cpp
#include "midi_monitor.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QDateTime>

namespace maestro::gui {

MidiMonitorWidget::MidiMonitorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void MidiMonitorWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    
    // Header
    auto* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel("MIDI Monitor"));
    
    auto* autoScrollCheck = new QCheckBox("Auto-scroll");
    autoScrollCheck->setChecked(true);
    connect(autoScrollCheck, &QCheckBox::toggled,
            this, &MidiMonitorWidget::onAutoScrollToggled);
    headerLayout->addWidget(autoScrollCheck);
    
    auto* clearBtn = new QPushButton("Clear");
    clearBtn->setFixedWidth(50);
    connect(clearBtn, &QPushButton::clicked,
            this, &MidiMonitorWidget::onClear);
    headerLayout->addWidget(clearBtn);
    
    mainLayout->addLayout(headerLayout);
    
    // Message list
    messageList_ = new QListWidget();
    messageList_->setAlternatingRowColors(true);
    messageList_->setStyleSheet(R"(
        QListWidget {
            background-color: #1a1a1a;
            color: #00ff00;
            font-family: monospace;
            font-size: 11px;
        }
        QListWidget::item:selected {
            background-color: #004400;
        }
    )");
    connect(messageList_, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        emit messageClicked(item->text());
    });
    mainLayout->addWidget(messageList_);
    
    // Detail view
    detailView_ = new QTextEdit();
    detailView_->setReadOnly(true);
    detailView_->setMaximumHeight(80);
    detailView_->setPlaceholderText("Message details will appear here...");
    mainLayout->addWidget(detailView_);
    
    // Status
    auto* statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet("color: #888; font-size: 10px;");
    mainLayout->addWidget(statusLabel);
}

void MidiMonitorWidget::addMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString fullMessage = QString("[%1] %2").arg(timestamp).arg(message);
    
    messageList_->addItem(fullMessage);
    messageCount_++;
    
    // Remove old messages if over limit
    while (messageList_->count() > maxMessages_) {
        delete messageList_->takeItem(0);
    }
    
    if (autoScroll_) {
        messageList_->scrollToBottom();
    }
    
    // Update detail view with last message
    detailView_->setText(fullMessage);
}

void MidiMonitorWidget::clearMessages() {
    messageList_->clear();
    detailView_->clear();
    messageCount_ = 0;
}

void MidiMonitorWidget::setMaxMessages(int max) {
    maxMessages_ = max;
}

void MidiMonitorWidget::startMonitoring() {
    // Monitoring started
}

void MidiMonitorWidget::stopMonitoring() {
    // Monitoring stopped
}

void MidiMonitorWidget::onClear() {
    clearMessages();
}

void MidiMonitorWidget::onAutoScrollToggled(bool enabled) {
    autoScroll_ = enabled;
}

} // namespace maestro::gui
