// src/gui/qt/widgets/terminal_widget.hpp
#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QProcess>
#include <QVBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>

namespace maestro::gui {

class TerminalWidget : public QWidget {
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget* parent = nullptr);
    ~TerminalWidget() override;

    void startInitialization();
    void appendLog(const QString& text, bool success = true);
    void appendError(const QString& text);
    void appendSuccess(const QString& text);
    void setProgress(int value);
    void setProgressRange(int min, int max);
    void setStatusMessage(const QString& message);

signals:
    void initializationComplete();
    void initializationFailed();

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onTimeout();

private:
    void setupUI();
    void runCommand(const QString& command, const QStringList& args = {});
    void runInitializationStep();
    void runNextCommand();
    QString getTimestamp() const;

    QTextEdit* logOutput_;
    QLabel* statusLabel_;
    QProgressBar* progressBar_;
    QLabel* iconLabel_;
    QVBoxLayout* mainLayout_;
    
    QProcess* currentProcess_;
    QTimer* timeoutTimer_;
    QStringList pendingCommands_;
    int currentStep_;
    int totalSteps_;
    
    bool success_;
};

} // namespace maestro::gui
