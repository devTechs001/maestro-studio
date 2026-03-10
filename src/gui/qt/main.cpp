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

#include "maestro/core/engine.hpp"
#include "maestro/core/settings.hpp"
#include "widgets/splash_screen.hpp"
#include "widgets/onboarding_wizard.hpp"
#include "widgets/settings_dialog.hpp"
#include "mainwindow.hpp"

using namespace maestro;
using namespace maestro::gui;

int main(int argc, char *argv[]) {
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
    
    // Show splash screen
    SplashScreen splash;
    splash.showMessage("Initializing audio engine...");
    splash.show();
    app.processEvents();
    
    // Initialize audio engine
    splash.showMessage("Loading audio drivers...");
    app.processEvents();
    
    MaestroEngine::Config engineConfig;
    engineConfig.sampleRate = SettingsManager::instance().audio().sampleRate;
    engineConfig.bufferSize = SettingsManager::instance().audio().bufferSize;
    
    auto& engine = MaestroEngine::instance();
    auto initResult = engine.initialize(engineConfig);
    
    splash.setProgress(30, 100);
    app.processEvents();
    
    splash.showMessage("Loading MIDI system...");
    app.processEvents();
    
    splash.setProgress(50, 100);
    app.processEvents();
    
    splash.showMessage("Loading instruments...");
    app.processEvents();
    
    splash.setProgress(70, 100);
    app.processEvents();
    
    splash.showMessage("Preparing user interface...");
    app.processEvents();
    
    splash.setProgress(90, 100);
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
    
    // Start engine
    engine.start();
    
    return app.exec();
}
