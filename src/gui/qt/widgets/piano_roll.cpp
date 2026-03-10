// src/gui/qt/widgets/piano_roll.cpp
#include "piano_roll.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <algorithm>

namespace maestro::gui {

PianoRollWidget::PianoRollWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Create scrollbars
    horizontalScroll_ = new QScrollBar(Qt::Horizontal, this);
    verticalScroll_ = new QScrollBar(Qt::Vertical, this);
    
    connect(horizontalScroll_, &QScrollBar::valueChanged, this, [this](int value) {
        drawOffsetX_ = value;
        update();
    });
    
    connect(verticalScroll_, &QScrollBar::valueChanged, this, [this](int value) {
        drawOffsetY_ = value;
        update();
    });
    
    // Initialize with some default notes
    addNote(60, 0, 480, 100);  // C4
    addNote(64, 480, 480, 100);  // E4
    addNote(67, 960, 480, 100);  // G4
}

void PianoRollWidget::setGridSize(int ticksPerBeat, int beatsPerBar) {
    ticksPerBeat_ = ticksPerBeat;
    beatsPerBar_ = beatsPerBar;
    update();
}

void PianoRollWidget::setNoteRange(int minNote, int maxNote) {
    minNote_ = minNote;
    maxNote_ = maxNote;
    verticalScroll_->setRange(0, (maxNote - minNote + 1) * noteHeight_ - height());
    update();
}

void PianoRollWidget::setTimeRange(int startTick, int endTick) {
    startTick_ = startTick;
    endTick_ = endTick;
    horizontalScroll_->setRange(0, (endTick - startTick) * pixelsPerTick_ - width());
    update();
}

void PianoRollWidget::addNote(int midiNote, int startTick, int length, int velocity) {
    NoteRect note;
    note.x = tickToX(startTick);
    note.y = noteToY(midiNote);
    note.width = std::max(10, length * pixelsPerTick_);
    note.height = noteHeight_ - 1;
    note.midiNote = midiNote;
    note.velocity = velocity;
    note.selected = false;
    
    notes_.push_back(note);
    noteSelection_.push_back(false);
    update();
}

void PianoRollWidget::removeNote(int index) {
    if (index >= 0 && index < static_cast<int>(notes_.size())) {
        notes_.erase(notes_.begin() + index);
        noteSelection_.erase(noteSelection_.begin() + index);
        update();
    }
}

void PianoRollWidget::selectNote(int index) {
    if (index >= 0 && index < static_cast<int>(notes_.size())) {
        deselectAll();
        notes_[index].selected = true;
        noteSelection_[index] = true;
        update();
    }
}

void PianoRollWidget::deselectAll() {
    for (auto& note : notes_) {
        note.selected = false;
    }
    std::fill(noteSelection_.begin(), noteSelection_.end(), false);
    update();
}

void PianoRollWidget::setPlaybackPosition(int tick) {
    playbackPosition_ = tick;
    update();
}

void PianoRollWidget::setLoopRegion(int startTick, int endTick) {
    loopStart_ = startTick;
    loopEnd_ = endTick;
    update();
}

void PianoRollWidget::zoomIn() {
    pixelsPerTick_ = std::min(pixelsPerTick_ * 2, 16);
    horizontalScroll_->setRange(0, (endTick_ - startTick_) * pixelsPerTick_ - width());
    update();
}

void PianoRollWidget::zoomOut() {
    pixelsPerTick_ = std::max(pixelsPerTick_ / 2, 1);
    horizontalScroll_->setRange(0, (endTick_ - startTick_) * pixelsPerTick_ - width());
    update();
}

void PianoRollWidget::zoomToFit() {
    pixelsPerTick_ = width() / (endTick_ - startTick_);
    update();
}

int PianoRollWidget::tickToX(int tick) const {
    return tick * pixelsPerTick_ - drawOffsetX_;
}

int PianoRollWidget::xToTick(int x) const {
    return (x + drawOffsetX_) / pixelsPerTick_;
}

int PianoRollWidget::noteToY(int midiNote) const {
    return (maxNote_ - midiNote) * noteHeight_ - drawOffsetY_;
}

int PianoRollWidget::yToNote(int y) const {
    return maxNote_ - (y + drawOffsetY_) / noteHeight_;
}

int PianoRollWidget::hitTestNote(int x, int y) const {
    for (size_t i = 0; i < notes_.size(); ++i) {
        const auto& note = notes_[i];
        if (x >= note.x && x <= note.x + note.width &&
            y >= note.y && y <= note.y + note.height) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void PianoRollWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    drawGrid(painter);
    drawNotes(painter);
    drawPlaybackHead(painter);
    drawKeyLabels(painter);
}

void PianoRollWidget::drawGrid(QPainter& painter) {
    painter.setPen(QColor(60, 60, 60));
    
    // Vertical lines (beats)
    int startBeat = xToTick(0) / ticksPerBeat_;
    int endBeat = xToTick(width()) / ticksPerBeat_ + 1;
    
    for (int beat = startBeat; beat <= endBeat; ++beat) {
        int x = tickToX(beat * ticksPerBeat_);
        painter.drawLine(x, 0, x, height());
    }
    
    // Horizontal lines (notes)
    for (int note = minNote_; note <= maxNote_; ++note) {
        int y = noteToY(note);
        painter.drawLine(0, y, width(), y);
        
        // Darker line for C notes
        if (note % 12 == 0) {
            painter.setPen(QColor(80, 80, 80));
            painter.drawLine(0, y, width(), y);
            painter.setPen(QColor(60, 60, 60));
        }
    }
}

void PianoRollWidget::drawNotes(QPainter& painter) {
    for (const auto& note : notes_) {
        // Note body
        if (note.selected) {
            painter.fillRect(note.x, note.y, note.width, note.height, QColor(100, 149, 237));
        } else {
            painter.fillRect(note.x, note.y, note.width, note.height, QColor(70, 130, 180));
        }
        
        // Note border
        painter.setPen(QColor(40, 80, 120));
        painter.drawRect(note.x, note.y, note.width, note.height);
    }
}

void PianoRollWidget::drawPlaybackHead(QPainter& painter) {
    int x = tickToX(playbackPosition_);
    painter.setPen(QPen(Qt::red, 2));
    painter.drawLine(x, 0, x, height());
}

void PianoRollWidget::drawKeyLabels(QPainter& painter) {
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 8));
    
    for (int note = minNote_; note <= maxNote_; ++note) {
        if (note % 12 == 0) {  // C notes only
            static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
            QString label = QString("%1%2").arg(noteNames[note % 12]).arg(note / 12 - 1);
            int y = noteToY(note);
            painter.drawText(2, y + 10, label);
        }
    }
}

void PianoRollWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int noteIndex = hitTestNote(event->pos().x(), event->pos().y());
        
        if (noteIndex >= 0) {
            // Select and start dragging existing note
            selectNote(noteIndex);
            dragging_ = true;
            draggedNoteIndex_ = noteIndex;
            dragStartX_ = event->pos().x();
            dragStartY_ = event->pos().y();
        } else {
            // Add new note
            int midiNote = yToNote(event->pos().y());
            int tick = xToTick(event->pos().x());
            addNote(midiNote, tick, ticksPerBeat_, 100);
            emit noteAdded(midiNote, tick, ticksPerBeat_, 100);
        }
    }
}

void PianoRollWidget::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_ && draggedNoteIndex_ >= 0) {
        int dx = event->pos().x() - dragStartX_;
        int dy = event->pos().y() - dragStartY_;
        
        auto& note = notes_[draggedNoteIndex_];
        note.x += dx;
        note.y += dy;
        
        dragStartX_ = event->pos().x();
        dragStartY_ = event->pos().y();
        
        update();
    }
}

void PianoRollWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (dragging_ && draggedNoteIndex_ >= 0) {
        auto& note = notes_[draggedNoteIndex_];
        int newNote = yToNote(note.y);
        int newTick = xToTick(note.x);
        
        emit noteModified(draggedNoteIndex_, newNote, newTick, note.width / pixelsPerTick_, note.velocity);
    }
    
    dragging_ = false;
    draggedNoteIndex_ = -1;
}

void PianoRollWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    } else {
        horizontalScroll_->setValue(horizontalScroll_->value() - event->angleDelta().y() / 10);
    }
}

void PianoRollWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            for (int i = static_cast<int>(notes_.size()) - 1; i >= 0; --i) {
                if (noteSelection_[i]) {
                    emit noteRemoved(i);
                    removeNote(i);
                }
            }
            break;
            
        case Qt::Key_Up:
            verticalScroll_->setValue(verticalScroll_->value() - 20);
            break;
            
        case Qt::Key_Down:
            verticalScroll_->setValue(verticalScroll_->value() + 20);
            break;
            
        case Qt::Key_Left:
            horizontalScroll_->setValue(horizontalScroll_->value() - 20);
            break;
            
        case Qt::Key_Right:
            horizontalScroll_->setValue(horizontalScroll_->value() + 20);
            break;
    }
}

void PianoRollWidget::resizeEvent(QResizeEvent* event) {
    horizontalScroll_->setGeometry(0, height() - 16, width() - 16, 16);
    verticalScroll_->setGeometry(width() - 16, 0, 16, height() - 16);
    QWidget::resizeEvent(event);
}

} // namespace maestro::gui
