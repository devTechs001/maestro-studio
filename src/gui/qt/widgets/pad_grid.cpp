// src/gui/qt/widgets/pad_grid.cpp
#include "pad_grid.hpp"
#include <QMouseEvent>
#include <QPainter>
#include <QMenu>

namespace maestro::gui {

PadButton::PadButton(int index, QWidget* parent)
    : QPushButton(parent)
    , index_(index)
{
    setFixedSize(80, 80);
    setText(QString::number(index + 1));
    setStyleSheet(R"(
        QPushButton {
            background-color: #404040;
            border: 2px solid #606060;
            border-radius: 8px;
            color: white;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QPushButton:pressed {
            background-color: #606060;
        }
    )");
}

void PadButton::setColor(const QColor& color) {
    color_ = color;
    update();
}

void PadButton::setVelocity(int velocity) {
    velocity_ = velocity;
    update();
}

void PadButton::setLabel(const QString& label) {
    setText(label);
}

void PadButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        pressed_ = true;
        // Calculate velocity from Y position
        int velocity = 127 - (event->pos().y() * 127 / height());
        velocity = qBound(1, velocity, 127);
        emit padPressed(index_, velocity);
        update();
    }
    QPushButton::mousePressEvent(event);
}

void PadButton::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        pressed_ = false;
        emit padReleased(index_);
        update();
    }
    QPushButton::mouseReleaseEvent(event);
}

void PadButton::paintEvent(QPaintEvent* event) {
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw velocity indicator
    if (pressed_ && velocity_ > 0) {
        QColor indicatorColor = color_;
        indicatorColor.setAlpha(velocity_ * 2);
        painter.fillRect(rect().adjusted(4, 4, -4, -4), indicatorColor);
    } else {
        // Draw normal color indicator
        QColor indicatorColor = color_;
        indicatorColor.setAlpha(80);
        painter.fillRect(rect().adjusted(4, 4, -4, -4), indicatorColor);
    }
}

// PadGridWidget implementation

PadGridWidget::PadGridWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void PadGridWidget::setupUI() {
    layout_ = new QGridLayout(this);
    layout_->setSpacing(4);
    layout_->setContentsMargins(8, 8, 8, 8);

    rebuildGrid();
}

void PadGridWidget::setGridSize(int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
    rebuildGrid();
}

void PadGridWidget::rebuildGrid() {
    // Clear existing pads
    for (auto& pad : pads_) {
        layout_->removeWidget(pad.get());
    }
    pads_.clear();

    // Create new pads
    for (int row = 0; row < rows_; ++row) {
        for (int col = 0; col < cols_; ++col) {
            int index = row * cols_ + col;
            auto pad = std::make_unique<PadButton>(index, this);

            connect(pad.get(), &PadButton::padPressed,
                    this, &PadGridWidget::onPadPressed);
            connect(pad.get(), &PadButton::padReleased,
                    this, &PadGridWidget::onPadReleased);

            // Right-click for assignment
            pad->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(pad.get(), &QWidget::customContextMenuRequested,
                    [this, index](const QPoint& pos) {
                        emit padAssignRequested(index);
                    });

            layout_->addWidget(pad.get(), row, col);
            pads_.push_back(std::move(pad));
        }
    }
}

void PadGridWidget::setPadColor(int index, const QColor& color) {
    if (index >= 0 && index < static_cast<int>(pads_.size())) {
        pads_[index]->setColor(color);
    }
}

void PadGridWidget::setPadLabel(int index, const QString& label) {
    if (index >= 0 && index < static_cast<int>(pads_.size())) {
        pads_[index]->setLabel(label);
    }
}

void PadGridWidget::setPadAssignment(int index, int type, int value) {
    // Pad assignment implementation
}

void PadGridWidget::triggerPad(int index, int velocity) {
    if (index >= 0 && index < static_cast<int>(pads_.size())) {
        pads_[index]->setVelocity(velocity);
    }
}

void PadGridWidget::releasePad(int index) {
    if (index >= 0 && index < static_cast<int>(pads_.size())) {
        pads_[index]->setVelocity(0);
    }
}

void PadGridWidget::onPadPressed(int index, int velocity) {
    emit padTriggered(index, velocity);
}

void PadGridWidget::onPadReleased(int index) {
    emit padReleased(index);
}

} // namespace maestro::gui
