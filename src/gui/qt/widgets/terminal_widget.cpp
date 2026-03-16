// src/gui/qt/widgets/terminal_widget.cpp
#include "terminal_widget.hpp"
#include <QFont>
#include <QScrollBar>
#include <QDateTime>
#include <QApplication>
#include <QScreen>

namespace maestro::gui {

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
    , currentProcess_(nullptr)
    , timeoutTimer_(new QTimer(this))
    , currentStep_(0)
    , totalSteps_(0)
    , success_(true)
{
    setupUI();
    
    connect(timeoutTimer_, &QTimer::timeout, this, &TerminalWidget::onTimeout);
    timeoutTimer_->setSingleShot(true);
    timeoutTimer_->setInterval(30000); // 30 second timeout
}

TerminalWidget::~TerminalWidget() {
    if (currentProcess_) {
        currentProcess_->kill();
        currentProcess_->deleteLater();
    }
}

void TerminalWidget::setupUI() {
    setMinimumSize(800, 600);
    setWindowTitle("MaestroStudio - Initialization");

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setSpacing(10);
    mainLayout_->setContentsMargins(20, 20, 20, 20);

    // Title
    auto* titleLabel = new QLabel("🎹 MaestroStudio - System Initialization");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6200EE;");
    mainLayout_->addWidget(titleLabel);

    // Icon
    iconLabel_ = new QLabel("⚙️");
    iconLabel_->setAlignment(Qt::AlignCenter);
    iconLabel_->setStyleSheet("font-size: 64px;");
    mainLayout_->addWidget(iconLabel_);

    // Status
    statusLabel_ = new QLabel("Initializing system components...");
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet("font-size: 14px; color: #666666;");
    mainLayout_->addWidget(statusLabel_);

    // Progress bar
    progressBar_ = new QProgressBar();
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setFormat("%p%");
    progressBar_->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #6200EE;
            border-radius: 5px;
            background: #f0f0f0;
            text-align: center;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #6200EE, stop:1 #03DAC6);
        }
    )");
    mainLayout_->addWidget(progressBar_);

    // Terminal output
    logOutput_ = new QTextEdit();
    logOutput_->setReadOnly(true);
    logOutput_->setFont(QFont("Monospace", 10));
    logOutput_->setStyleSheet(R"(
        QTextEdit {
            background: #1e1e1e;
            color: #00ff00;
            border: 2px solid #333;
            border-radius: 5px;
            padding: 10px;
        }
    )");
    logOutput_->setMinimumHeight(300);
    mainLayout_->addWidget(logOutput_);

    appendLog("═══════════════════════════════════════════════════════════", true);
    appendLog("  MaestroStudio v1.0.0 - System Initialization", true);
    appendLog("═══════════════════════════════════════════════════════════", true);
    appendLog("", true);
}

void TerminalWidget::startInitialization() {
    currentStep_ = 0;
    totalSteps_ = 8;
    setProgress(0);
    
    // Queue initialization commands
    pendingCommands_ = {
        "Checking audio subsystem (ALSA/PulseAudio/JACK)...",
        "Checking MIDI subsystem...",
        "Loading audio engine components...",
        "Loading MIDI engine components...",
        "Loading instrument definitions...",
        "Loading style engine...",
        "Loading voice engine...",
        "Finalizing initialization..."
    };
    
    // Simulate initialization steps
    runInitializationStep();
}

void TerminalWidget::runInitializationStep() {
    if (currentStep_ >= pendingCommands_.size()) {
        // All steps complete
        iconLabel_->setText("✅");
        statusLabel_->setText("Initialization Complete!");
        setProgress(100);
        appendSuccess("═══════════════════════════════════════════════════════════");
        appendSuccess("  All systems initialized successfully!");
        appendSuccess("═══════════════════════════════════════════════════════════");
        
        QTimer::singleShot(1500, this, [this]() {
            emit initializationComplete();
        });
        return;
    }
    
    const QString& step = pendingCommands_[currentStep_];
    setStatusMessage(step);
    appendLog(QString("[%1/%2] %3").arg(currentStep_ + 1).arg(totalSteps_).arg(step), true);
    
    // Simulate work being done
    QTimer::singleShot(800, this, [this]() {
        setProgress(static_cast<int>((currentStep_ + 1) * 100.0 / totalSteps_));
        currentStep_++;
        runInitializationStep();
    });
}

void TerminalWidget::appendLog(const QString& text, bool success) {
    QColor color = success ? QColor("#00ff00") : QColor("#ff6b6b");
    QString timestamp = getTimestamp();
    
    logOutput_->setTextColor(color);
    logOutput_->append(QString("[%1] %2").arg(timestamp).arg(text));
    logOutput_->verticalScrollBar()->setValue(logOutput_->verticalScrollBar()->maximum());
    
    QApplication::processEvents();
}

void TerminalWidget::appendError(const QString& text) {
    logOutput_->setTextColor(QColor("#ff4444"));
    logOutput_->append(QString("[ERROR] %1").arg(text));
    logOutput_->verticalScrollBar()->setValue(logOutput_->verticalScrollBar()->maximum());
    success_ = false;
}

void TerminalWidget::appendSuccess(const QString& text) {
    logOutput_->setTextColor(QColor("#00ff88"));
    logOutput_->append(text);
    logOutput_->verticalScrollBar()->setValue(logOutput_->verticalScrollBar()->maximum());
}

void TerminalWidget::setProgress(int value) {
    progressBar_->setValue(value);
    QApplication::processEvents();
}

void TerminalWidget::setProgressRange(int min, int max) {
    progressBar_->setRange(min, max);
}

void TerminalWidget::setStatusMessage(const QString& message) {
    statusLabel_->setText(message);
    QApplication::processEvents();
}

void TerminalWidget::onProcessReadyRead() {
    if (currentProcess_) {
        QString output = QString::fromUtf8(currentProcess_->readAllStandardOutput());
        if (!output.trimmed().isEmpty()) {
            appendLog(output.trimmed(), true);
        }
        
        QString error = QString::fromUtf8(currentProcess_->readAllStandardError());
        if (!error.trimmed().isEmpty()) {
            appendError(error.trimmed());
        }
    }
}

void TerminalWidget::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    timeoutTimer_->stop();

    if (status == QProcess::NormalExit && exitCode == 0) {
        appendSuccess("✓ Command completed successfully");
    } else {
        appendError(QString("✗ Command failed with exit code %1").arg(exitCode));
    }

    if (currentProcess_) {
        currentProcess_->deleteLater();
        currentProcess_ = nullptr;
    }

    // Run next command
    if (!pendingCommands_.isEmpty()) {
        QTimer::singleShot(100, this, [this]() {
            runNextCommand();
        });
    } else if (success_) {
        emit initializationComplete();
    } else {
        emit initializationFailed();
    }
}

void TerminalWidget::runNextCommand() {
    if (!pendingCommands_.isEmpty()) {
        QString cmd = pendingCommands_.takeFirst();
        runCommand("/bin/sh", QStringList() << "-c" << cmd);
    }
}

void TerminalWidget::onTimeout() {
    if (currentProcess_) {
        currentProcess_->kill();
        appendError("Command timed out after 30 seconds");
        onProcessFinished(-1, QProcess::CrashExit);
    }
}

void TerminalWidget::runCommand(const QString& command, const QStringList& args) {
    appendLog(QString("Executing: %1 %2").arg(command).arg(args.join(" ")), true);
    
    currentProcess_ = new QProcess(this);
    connect(currentProcess_, &QProcess::readyReadStandardOutput, this, &TerminalWidget::onProcessReadyRead);
    connect(currentProcess_, &QProcess::readyReadStandardError, this, &TerminalWidget::onProcessReadyRead);
    connect(currentProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalWidget::onProcessFinished);
    
    currentProcess_->start(command, args);
    timeoutTimer_->start();
}

QString TerminalWidget::getTimestamp() const {
    return QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
}

} // namespace maestro::gui
