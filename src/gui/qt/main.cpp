// src/gui/qt/main.cpp
#include <QApplication>
#include <QMainWindow>
#include <QSplashScreen>
#include <QTimer>
#include <QSettings>
#include <QStyleFactory>
#include <QFont>
#include <QFile>
#include <QDir>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QThread>

#include "maestro/core/engine.hpp"
#include "maestro/core/settings.hpp"
#include "widgets/splash_screen.hpp"
#include "widgets/onboarding_wizard.hpp"
#include "widgets/settings_dialog.hpp"
#include "widgets/terminal_widget.hpp"
#include "mainwindow.hpp"

using namespace maestro;
using namespace maestro::gui;

int main(int argc, char *argv[]) {
    // Set environment for headless systems
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    // Set high DPI attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    // Set application info
    app.setApplicationName("MaestroStudio");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MaestroStudio");
    app.setApplicationDisplayName("MaestroStudio");

    // Initialize settings manager
    SettingsManager::instance().initialize();

    // Load theme from settings
    auto& interfaceSettings = SettingsManager::instance().interface();

    // Apply stylesheet
    QFile styleFile(":/styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
    }

    // Set default font
    QFont defaultFont(interfaceSettings.fontName.c_str(), interfaceSettings.fontSize);
    app.setFont(defaultFont);

    // Show terminal initialization widget
    TerminalWidget terminalWidget;
    terminalWidget.show();
    app.processEvents();

    // Start initialization process
    QTimer::singleShot(500, [&terminalWidget]() {
        terminalWidget.startInitialization();
    });

    // Wait for initialization to complete
    bool initComplete = false;
    bool initFailed = false;

    QObject::connect(&terminalWidget, &TerminalWidget::initializationComplete,
        [&initComplete]() { initComplete = true; });
    
    QObject::connect(&terminalWidget, &TerminalWidget::initializationFailed,
        [&initFailed]() { initFailed = true; });

    // Process events until initialization completes
    while (!initComplete && !initFailed) {
        app.processEvents();
        QThread::msleep(50);
    }

    terminalWidget.close();

    if (initFailed) {
        qCritical() << "Initialization failed!";
        return -1;
    }

    // Show splash screen
    SplashScreen splash;
    splash.showMessage("Finalizing system...");
    splash.show();
    app.processEvents();

    // Initialize audio engine
    MaestroEngine::Config engineConfig;
    engineConfig.sampleRate = SettingsManager::instance().audio().sampleRate;
    engineConfig.bufferSize = SettingsManager::instance().audio().bufferSize;

    auto& engine = MaestroEngine::instance();
    auto initResult = engine.initialize(engineConfig);
    
    if (!initResult.isSuccess()) {
        qWarning() << "Audio engine initialization warning:" << QString::fromStdString(initResult.error());
        // Continue anyway - audio will be unavailable
    }

    splash.setProgress(30, 100);
    app.processEvents();

    splash.showMessage("Starting audio engine...");
    app.processEvents();

    auto startResult = engine.start();
    if (!startResult.isSuccess()) {
        qWarning() << "Audio engine start warning:" << QString::fromStdString(startResult.error());
        // Continue anyway - audio will be unavailable
    }

    splash.setProgress(60, 100);
    app.processEvents();

    splash.showMessage("Loading user interface...");
    app.processEvents();

    splash.setProgress(80, 100);
    app.processEvents();

    // Check if onboarding should be shown
    bool showOnboarding = interfaceSettings.showOnboarding;

    // Create main window
    MainWindow mainWindow;

    splash.setProgress(100, 100);
    app.processEvents();

    // Finish splash with fade effect
    QTimer::singleShot(500, [&splash, &mainWindow]() {
        splash.finishWithFade(&mainWindow);
    });

    // Show onboarding if needed
    if (showOnboarding) {
        OnboardingWizard wizard(&mainWindow);
        wizard.connect(&wizard, &OnboardingWizard::onboardingCompleted,
            [](const QVariantMap& settings) {
                // Apply onboarding settings
                auto& audioSettings = SettingsManager::instance().audio();
                audioSettings.driver = settings["audioDriver"].toString().toStdString();
                audioSettings.sampleRate = settings["sampleRate"].toInt();
                audioSettings.bufferSize = settings["bufferSize"].toInt();

                SettingsManager::instance().save();
            });

        QTimer::singleShot(1000, [&wizard]() {
            wizard.exec();
        });
    }

    // Show main window
    mainWindow.show();

    return app.exec();
}
