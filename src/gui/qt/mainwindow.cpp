// src/gui/qt/mainwindow.cpp
#include "mainwindow.hpp"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>

namespace maestro::gui {

// Forward declare widget classes (stubs for compilation)
class PianoRollWidget : public QWidget {
public:
    PianoRollWidget(QWidget* parent) : QWidget(parent) {}
};

class MixerWidget : public QWidget {
public:
    MixerWidget(QWidget* parent) : QWidget(parent) {
        setMinimumHeight(150);
    }
};

class PadGridWidget : public QWidget {
public:
    PadGridWidget(QWidget* parent) : QWidget(parent) {
        setMinimumSize(300, 300);
    }
};

class StyleControlWidget : public QWidget {
public:
    StyleControlWidget(QWidget* parent) : QWidget(parent) {
        setMinimumWidth(200);
    }
};

class VoiceSelectWidget : public QWidget {
public:
    VoiceSelectWidget(QWidget* parent) : QWidget(parent) {
        setMinimumWidth(200);
    }
};

class RegistrationWidget : public QWidget {
public:
    RegistrationWidget(QWidget* parent) : QWidget(parent) {
        setMinimumWidth(200);
    }
};

class TransportWidget : public QWidget {
public:
    TransportWidget(QWidget* parent) : QWidget(parent) {
        setFixedHeight(40);
    }
};

class MidiMonitorWidget : public QWidget {
public:
    MidiMonitorWidget(QWidget* parent) : QWidget(parent) {}
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("MaestroStudio");
    setMinimumSize(1280, 720);

    setupUI();
    setupMenus();
    setupToolbars();
    setupDockWidgets();
    setupConnections();
    loadSettings();
}

MainWindow::~MainWindow() {
    saveSettings();
}

void MainWindow::setupUI() {
    // Central widget with tabs
    centralTabs_ = new QTabWidget(this);
    
    // Add piano roll tab
    auto* pianoRoll = new PianoRollWidget(this);
    centralTabs_->addTab(pianoRoll, "Piano Roll");
    
    // Add arranger tab
    auto* arranger = new QWidget(this);
    centralTabs_->addTab(arranger, "Arranger");
    
    setCentralWidget(centralTabs_);

    // Status bar
    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenus() {
    // File menu
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&New Project"), this, &MainWindow::onNewProject,
                       QKeySequence::New);
    fileMenu->addAction(tr("&Open Project..."), this, &MainWindow::onOpenProject,
                       QKeySequence::Open);
    fileMenu->addAction(tr("&Save Project"), this, &MainWindow::onSaveProject,
                       QKeySequence::Save);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QMainWindow::close, QKeySequence::Quit);

    // Edit menu
    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Undo"), QKeySequence::Undo);
    editMenu->addAction(tr("&Redo"), QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction(tr("Cu&t"), QKeySequence::Cut);
    editMenu->addAction(tr("&Copy"), QKeySequence::Copy);
    editMenu->addAction(tr("&Paste"), QKeySequence::Paste);

    // View menu
    auto* viewMenu = menuBar()->addMenu(tr("&View"));

    // Transport menu
    auto* transportMenu = menuBar()->addMenu(tr("&Transport"));
    transportMenu->addAction(tr("&Play"), this, &MainWindow::onPlay,
                            Qt::Key_Space);
    transportMenu->addAction(tr("&Stop"), this, &MainWindow::onStop);
    transportMenu->addAction(tr("&Record"), this, &MainWindow::onRecord,
                            Qt::Key_R);

    // MIDI menu
    auto* midiMenu = menuBar()->addMenu(tr("&MIDI"));
    midiMenu->addAction(tr("MIDI &Devices..."));
    midiMenu->addAction(tr("MIDI &Learn"));

    // Instruments menu
    auto* instrumentsMenu = menuBar()->addMenu(tr("&Instruments"));
    instrumentsMenu->addAction(tr("&Connect Instrument..."));

    // Help menu
    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About MaestroStudio"));

    // View menu created (dock toggles added in setupDockWidgets)
    menuBar()->addMenu(tr("&View"));
}

void MainWindow::setupToolbars() {
    // Transport toolbar
    auto* transportBar = addToolBar(tr("Transport"));
    transportBar->setObjectName("TransportToolbar");

    transportWidget_ = new TransportWidget(this);
    transportBar->addWidget(transportWidget_);
}

void MainWindow::setupDockWidgets() {
    setDockNestingEnabled(true);

    // Mixer dock (bottom)
    mixerDock_ = new QDockWidget(tr("Mixer"), this);
    mixerDock_->setObjectName("MixerDock");
    mixerWidget_ = new MixerWidget(this);
    mixerDock_->setWidget(mixerWidget_);
    addDockWidget(Qt::BottomDockWidgetArea, mixerDock_);

    // Pads dock (right)
    padsDock_ = new QDockWidget(tr("Pads"), this);
    padsDock_->setObjectName("PadsDock");
    padWidget_ = new PadGridWidget(this);
    padsDock_->setWidget(padWidget_);
    addDockWidget(Qt::RightDockWidgetArea, padsDock_);

    // Style control dock (right)
    styleDock_ = new QDockWidget(tr("Style Control"), this);
    styleDock_->setObjectName("StyleDock");
    styleWidget_ = new StyleControlWidget(this);
    styleDock_->setWidget(styleWidget_);
    addDockWidget(Qt::RightDockWidgetArea, styleDock_);

    // Voices dock (left)
    voicesDock_ = new QDockWidget(tr("Voices"), this);
    voicesDock_->setObjectName("VoicesDock");
    voiceWidget_ = new VoiceSelectWidget(this);
    voicesDock_->setWidget(voiceWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, voicesDock_);

    // Registration dock (left)
    registrationDock_ = new QDockWidget(tr("Registrations"), this);
    registrationDock_->setObjectName("RegistrationDock");
    regWidget_ = new RegistrationWidget(this);
    registrationDock_->setWidget(regWidget_);
    addDockWidget(Qt::LeftDockWidgetArea, registrationDock_);

    // MIDI Monitor dock (right, hidden by default)
    midiMonitorDock_ = new QDockWidget(tr("MIDI Monitor"), this);
    midiMonitorDock_->setObjectName("MidiMonitorDock");
    midiMonitorWidget_ = new MidiMonitorWidget(this);
    midiMonitorDock_->setWidget(midiMonitorWidget_);
    addDockWidget(Qt::RightDockWidgetArea, midiMonitorDock_);
    midiMonitorDock_->hide();

    // Tab some docks together
    tabifyDockWidget(padsDock_, styleDock_);
    tabifyDockWidget(voicesDock_, registrationDock_);

    // Add dock widget toggles to View menu (after docks are created)
    auto* viewMenu = menuBar()->findChild<QMenu*>("View");
    if (!viewMenu) {
        viewMenu = menuBar()->addMenu(tr("&View"));
    }
    viewMenu->addAction(mixerDock_->toggleViewAction());
    viewMenu->addAction(padsDock_->toggleViewAction());
    viewMenu->addAction(styleDock_->toggleViewAction());
    viewMenu->addAction(voicesDock_->toggleViewAction());
    viewMenu->addAction(registrationDock_->toggleViewAction());
    viewMenu->addAction(midiMonitorDock_->toggleViewAction());
}

void MainWindow::setupConnections() {
    // Connect engine signals/slots here
}

void MainWindow::loadSettings() {
    QSettings settings("MaestroStudio", "MaestroStudio");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::saveSettings() {
    QSettings settings("MaestroStudio", "MaestroStudio");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::onNewProject() {
    // New project implementation
}

void MainWindow::onOpenProject() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open Project"), QString(),
        tr("MaestroStudio Projects (*.msp);;All Files (*)")
    );

    if (!path.isEmpty()) {
        emit projectLoaded(path);
    }
}

void MainWindow::onSaveProject() {
    // Save project implementation
}

void MainWindow::onExportAudio() {
    // Export audio implementation
}

void MainWindow::onExportMidi() {
    // Export MIDI implementation
}

void MainWindow::onPlay() {
    // Play implementation
}

void MainWindow::onStop() {
    // Stop implementation
}

void MainWindow::onRecord() {
    // Record implementation
}

} // namespace maestro::gui
