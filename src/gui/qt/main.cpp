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

    // Keep terminal widget visible - don't proceed further
    return app.exec();
}
