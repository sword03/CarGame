#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>

// Crosslink between Qt class and win callback
MainWindow * mwReference;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        // 屏蔽win键，printscreen键, ALT键（WM_SYSKEYDOWN)等
        PKBDLLHOOKSTRUCT p = reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam);
        if(p->vkCode == VK_SNAPSHOT) return 1; //block print-screen key
        if(p->vkCode == VK_LWIN) return 1; //block print-screen key
        if(p->vkCode == VK_RWIN) return 1; //block print-screen key
        if(wParam == WM_SYSKEYDOWN) return 1; //block print-screen key
        switch (wParam)
        {
            // Pass KeyDown/KeyUp messages for Qt class to logicize
            case WM_KEYDOWN:
                mwReference->keyDown(PKBDLLHOOKSTRUCT(lParam)->vkCode);
            break;
            case WM_KEYUP:
                mwReference->keyUp(PKBDLLHOOKSTRUCT(lParam)->vkCode);
            break;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    carPos.setX(0);
    carPos.setY(0);

    tempPix = QPixmap(5000, 5000);

    ui->setupUi(this);

    // Setup variables
    mwReference = this;
    bWinKey     = false;

    // Install the low-level keyboard & mouse hooks
    hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
}

MainWindow::~MainWindow()
{
    // Be friendly! Remove hooks!
    UnhookWindowsHookEx(hhkLowLevelKybd);
    delete ui;
}

void MainWindow::keyDown(DWORD key)
{
    // This section gets launched for every key pressed ever
    // Filter out the keys we are interested
    // Launch doMultimedia everytime key of interest is pressed
    //      Otherwise it'll still launch even if non-interested key is pressed
    //      Also will result in massive trigger spam when right keys are pressed
    //      By keeping it within the parenthesis, doMultimedia will fire on that specific key repeat
    //      Otherwise it'll spam on both win and key repeat, meaning it'll fire 30+ a second if outside brackets
    // qInfo() << key;
    carPos.setX(carPos.x() + 10);
    // carPos.setY(carPos.y() + 1);

    int right, bottom;
    right = rect().width();
    bottom = rect().height();
    if(carPos.x() > right){
        carPos.setX(-320);
        carPos.setY(carPos.y() + 200);
    }
    if(carPos.y() + 280 > bottom){
        carPos.setY(0);
    }
    update();
    if(key == VK_LWIN || key == VK_RWIN)
    {
        bWinKey = true;
        doMultimedia(key);
    }
    if(key == VK_F9)
        doMultimedia(key);
    if(key == VK_F10)
        doMultimedia(key);
    if(key == VK_DOWN)
        doMultimedia(key);
    if(key == VK_UP)
        doMultimedia(key);
    if(key == VK_END)
        doMultimedia(key);
}

void MainWindow::keyUp(DWORD key)
{
    // Remove the bools
    if(key == VK_LWIN || key == VK_RWIN)
        bWinKey = false;
}

void MainWindow::pressKey(DWORD vkKeyCode)
{
    INPUT Input;
    // Set up a generic keyboard event.
    Input.type = INPUT_KEYBOARD;
    Input.ki.wScan = 0;
    Input.ki.time = 0;
    Input.ki.dwExtraInfo = 0;
    Input.ki.dwFlags = 0;

    Input.ki.wVk = vkKeyCode;
    SendInput(1, &Input, sizeof(INPUT));
}

void MainWindow::doMultimedia(DWORD vkKeyCode)
{
    // Check if any win key is being pressed first
    if(!bWinKey) return;

    // Play / Pause
    if (vkKeyCode == VK_F9)
        pressKey(VK_MEDIA_PLAY_PAUSE);
    // Next Track
    else if (vkKeyCode == VK_F10)
        pressKey(VK_MEDIA_NEXT_TRACK);
    // Lower Volume
    else if (vkKeyCode == VK_DOWN)
        pressKey(VK_VOLUME_DOWN);
    // Increase Volume
    else if (vkKeyCode == VK_UP)
        pressKey(VK_VOLUME_UP);
    // Mute
    else if (vkKeyCode == VK_END)
        pressKey(VK_VOLUME_MUTE);
}

void MainWindow::setPainterForCar(QPainter & painter){
    QPen pen; //画笔
    pen.setColor(QColor(0, 255, 0));
    QBrush brush(QColor(255, 0, 0, 125)); //画刷
    painter.setPen(pen); //添加画笔
    painter.setBrush(brush); //添加画刷
}

void MainWindow::setPainterForDoor(QPainter & painter){
    QPen pen; //画笔
    pen.setColor(QColor(0, 255, 0));
    QBrush brush(QColor(255, 255, 255, 255)); //画刷
    painter.setPen(pen); //添加画笔
    painter.setBrush(brush); //添加画刷
}

void MainWindow::setPainterForTire(QPainter & painter){
    QPen pen; //画笔
    pen.setColor(QColor(0, 255, 0));
    QBrush brush(QColor(0, 255, 255, 255)); //画刷
    painter.setPen(pen); //添加画笔
    painter.setBrush(brush); //添加画刷
}

QPoint MainWindow::absPos(QPoint pos){
    return QPoint(carPos.x() + pos.x(), carPos.y() + pos.y());
}

QRect MainWindow::absRect(QRect rect){
    return QRect(carPos.x() + rect.left(), carPos.y() + rect.top(), rect.width(), rect.height());
}

void MainWindow::paintEvent(QPaintEvent *)
{
    // 双缓冲
    tempPix.fill(QColor(128, 128, 128));
    QPainter memPainter(&tempPix);
    //QPainter memPainter(this);
    // 车身
    setPainterForCar(memPainter);
    memPainter.drawRect(absRect(QRect(50, 50, 200, 100))); //绘制矩形

    // 车头
    QPoint points[5] = {
        absPos(QPoint(260, 50)),
        absPos(QPoint(260 + 30, 50)),
        absPos(QPoint(260 + 110, 110)),
        absPos(QPoint(260 + 110, 150)),
        absPos(QPoint(260, 150))
    };
    memPainter.drawPolygon(points, 5);

    // 车轮
    setPainterForTire(memPainter);
    QPainterPath path;
    path.addEllipse(absRect(QRect(50, 170, 50, 50)));
    path.addEllipse(absRect(QRect(180, 170, 50, 50)));
    path.addEllipse(absRect(QRect(280, 170, 50, 50)));
    memPainter.drawPath(path);


    // 车门
    setPainterForDoor(memPainter);
    QPoint points2[5] = {
        absPos(QPoint(270, 70)),
        absPos(QPoint(270 + 25, 70)),
        absPos(QPoint(270 + 50, 90)),
        absPos(QPoint(270 + 50, 140)),
        absPos(QPoint(270, 140))
    };
    memPainter.drawPolygon(points2, 5);

    QPainter painter(this);
    painter.drawPixmap(0, 0, tempPix);
}
