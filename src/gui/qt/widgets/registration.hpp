// src/gui/qt/widgets/registration.hpp
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <vector>

namespace maestro::gui {

class RegistrationWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegistrationWidget(QWidget* parent = nullptr);

    void setBank(int bank);
    int currentBank() const;
    
    void setRegistrationName(int index, const QString& name);
    QString registrationName(int index) const;

signals:
    void registrationSelected(int bank, int memory);
    void bankChanged(int bank);

private slots:
    void onBankUp();
    void onBankDown();
    void onMemoryClicked(int index);

private:
    void setupUI();
    void updateDisplay();

    QLabel* bankLabel_;
    QLabel* currentDisplay_;
    std::vector<QPushButton*> memoryButtons_;
    
    int currentBank_ = 0;
    int currentMemory_ = 0;
};

} // namespace maestro::gui
