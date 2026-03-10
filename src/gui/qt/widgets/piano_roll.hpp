// src/gui/qt/widgets/piano_roll.hpp
#pragma once

#include <QWidget>
#include <QScrollBar>
#include <vector>

namespace maestro::gui {

struct NoteRect {
    int x, y, width, height;
    int midiNote;
    int velocity;
    bool selected;
};

class PianoRollWidget : public QWidget {
    Q_OBJECT

public:
    explicit PianoRollWidget(QWidget* parent = nullptr);

    void setGridSize(int ticksPerBeat, int beatsPerBar);
    void setNoteRange(int minNote, int maxNote);
    void setTimeRange(int startTick, int endTick);
    
    // Note editing
    void addNote(int midiNote, int startTick, int length, int velocity);
    void removeNote(int index);
    void selectNote(int index);
    void deselectAll();
    
    // Playback
    void setPlaybackPosition(int tick);
    void setLoopRegion(int startTick, int endTick);
    
    // Zoom
    void zoomIn();
    void zoomOut();
    void zoomToFit();

signals:
    void noteAdded(int midiNote, int startTick, int length, int velocity);
    void noteRemoved(int index);
    void noteModified(int index, int midiNote, int startTick, int length, int velocity);
    void positionChanged(int tick);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void drawGrid(QPainter& painter);
    void drawNotes(QPainter& painter);
    void drawPlaybackHead(QPainter& painter);
    void drawKeyLabels(QPainter& painter);
    
    int tickToX(int tick) const;
    int xToTick(int x) const;
    int noteToY(int midiNote) const;
    int yToNote(int y) const;
    int hitTestNote(int x, int y) const;
    
    std::vector<NoteRect> notes_;
    std::vector<bool> noteSelection_;
    
    QScrollBar* horizontalScroll_;
    QScrollBar* verticalScroll_;
    
    int ticksPerBeat_ = 480;
    int beatsPerBar_ = 4;
    int minNote_ = 21;   // A0
    int maxNote_ = 108;  // C8
    int startTick_ = 0;
    int endTick_ = 480 * 16;  // 16 bars
    
    int playbackPosition_ = 0;
    int loopStart_ = 0;
    int loopEnd_ = 480 * 4;
    
    int pixelsPerTick_ = 2;
    int noteHeight_ = 12;
    
    bool dragging_ = false;
    int dragStartX_ = 0;
    int dragStartY_ = 0;
    int draggedNoteIndex_ = -1;
    int drawOffsetX_ = 0;
    int drawOffsetY_ = 0;
};

} // namespace maestro::gui
