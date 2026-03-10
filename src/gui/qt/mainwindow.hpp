// src/gui/qt/mainwindow.hpp
#pragma once

#include <QMainWindow>
#include <QDockWidget>
#include <QTabWidget>
#include <memory>

namespace maestro::gui {

class PianoRollWidget;
class MixerWidget;
class PadGridWidget;
class StyleControlWidget;
class VoiceSelectWidget;
class RegistrationWidget;
class TransportWidget;
class MidiMonitorWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void projectLoaded(const QString& path);
    void projectSaved(const QString& path);

private slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onExportAudio();
    void onExportMidi();
    void onPlay();
    void onStop();
    void onRecord();

private:
    void setupUI();
    void setupMenus();
    void setupToolbars();
    void setupDockWidgets();
    void setupConnections();
    void loadSettings();
    void saveSettings();

    // Central widget
    QTabWidget* centralTabs_;

    // Dock widgets
    QDockWidget* mixerDock_;
    QDockWidget* padsDock_;
    QDockWidget* styleDock_;
    QDockWidget* voicesDock_;
    QDockWidget* registrationDock_;
    QDockWidget* midiMonitorDock_;

    // Widget instances
    MixerWidget* mixerWidget_;
    PadGridWidget* padWidget_;
    StyleControlWidget* styleWidget_;
    VoiceSelectWidget* voiceWidget_;
    RegistrationWidget* regWidget_;
    TransportWidget* transportWidget_;
    MidiMonitorWidget* midiMonitorWidget_;
};

} // namespace maestro::gui
