// src/gui/qt/widgets/pad_grid.hpp
#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QColor>
#include <vector>
#include <memory>

namespace maestro::gui {

class PadButton : public QPushButton {
    Q_OBJECT

public:
    explicit PadButton(int index, QWidget* parent = nullptr);

    int index() const { return index_; }
    void setColor(const QColor& color);
    void setVelocity(int velocity);
    void setLabel(const QString& label);

signals:
    void padPressed(int index, int velocity);
    void padReleased(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    int index_;
    QColor color_ = Qt::gray;
    int velocity_ = 0;
    bool pressed_ = false;
};

class PadGridWidget : public QWidget {
    Q_OBJECT

public:
    explicit PadGridWidget(QWidget* parent = nullptr);

    void setGridSize(int rows, int cols);
    void setPadColor(int index, const QColor& color);
    void setPadLabel(int index, const QString& label);
    void setPadAssignment(int index, int type, int value);
    void triggerPad(int index, int velocity = 127);
    void releasePad(int index);

signals:
    void padTriggered(int index, int velocity);
    void padReleased(int index);
    void padAssignRequested(int index);

private slots:
    void onPadPressed(int index, int velocity);
    void onPadReleased(int index);

private:
    void setupUI();
    void rebuildGrid();

    QGridLayout* layout_;
    std::vector<std::unique_ptr<PadButton>> pads_;
    int rows_ = 4;
    int cols_ = 4;
};

} // namespace maestro::gui
