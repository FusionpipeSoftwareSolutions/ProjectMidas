#include "midasgui.h"
#include <string>
#include <Windows.h>

MidasGUI::MidasGUI(MidasThread *mainThread, QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    // set window as "always on top"
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

    // set window as transparent to keyboard/mouse events - not working right now. TODO
    HWND hwnd = (HWND)winId();
    LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_TRANSPARENT);
    styles = GetWindowLong(hwnd, GWL_EXSTYLE);

    this->mainThread = mainThread;

    connect(mainThread, SIGNAL(outputCount(int)), this, SLOT(handleCount(int)));

    qRegisterMetaType<std::string>("std::String");
    connect(mainThread, SIGNAL(emitString(std::string)), this, SLOT(displayMessage(std::string)));


    // now for a mockup of the real business!
    connect(mainThread, SIGNAL(emitXVeloc(int)), this, SLOT(handleXVeloc(int)));
    connect(mainThread, SIGNAL(emitYVeloc(int)), this, SLOT(handleYVeloc(int)));
}

MidasGUI::~MidasGUI()
{

}

void MidasGUI::handleCount(int count)
{
    ui.label->setText(QString::number(count));
}

void MidasGUI::displayMessage(std::string str)
{
    ui.label->setText(QString::fromStdString(str));
}

void MidasGUI::handleXVeloc(int veloc)
{
    int nVeloc = (veloc + 100) / 2;
    ui.horizontalSlider->setValue(nVeloc);
    ui.horProgress->setValue(veloc);
}

void MidasGUI::handleYVeloc(int veloc)
{
    int nVeloc = (veloc + 100) / 2;
    ui.verticalSlider->setValue(nVeloc);
    ui.vertProgress->setValue(veloc);
}