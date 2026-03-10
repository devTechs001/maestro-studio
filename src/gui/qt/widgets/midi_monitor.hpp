// src/gui/qt/widgets/midi_monitor.hpp
#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>

namespace maestro::gui {

class MidiMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit MidiMonitorWidget(QWidget* parent = nullptr);

    void addMessage(const QString& message);
    void clearMessages();
    void setMaxMessages(int max);
    void startMonitoring();
    void stopMonitoring();

signals:
    void messageClicked(const QString& message);

private slots:
    void onClear();
    void onAutoScrollToggled(bool enabled);

private:
    void setupUI();

    QListWidget* messageList_;
    QTextEdit* detailView_;
    int maxMessages_ = 100;
    bool autoScroll_ = true;
    int messageCount_ = 0;
};

} // namespace maestro::gui
