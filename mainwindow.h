#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtMultimedia/QSoundEffect>
#include <QPixmap>
#include <QFont>
#include <QFontDatabase>
#include <QPointF>
#include <QRandomGenerator>
#include <QList>
#include <QVector>
#include <QUrl>
#include <QMessageBox>

#include "camera.h"
#include "bullet.h"
#include "chicken.h"
#include "atlas.h"
#include "animation.h"
#include "timer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateGame();

public:
    void loadResources();
    void unloadResources();
    void checkCollisions();
    void fireWeapon();
    void gameOver();

    Ui::MainWindow *ui;
    QTimer *gameTimer;
    QElapsedTimer frameTimer;

    // 游戏资源
    Camera *camera;
    QPixmap texHeart;
    QPixmap texBullet;
    QPixmap texBattery;
    QPixmap texCrosshair;
    QPixmap texBackground;
    QPixmap texBarrelIdle;

    Atlas m_atlasBarrelFire;  // 重命名为 m_ 前缀，避免与全局变量冲突
    Atlas m_atlasChickenFast;
    Atlas m_atlasChickenMedium;
    Atlas m_atlasChickenSlow;
    Atlas m_atlasExplosion;

    QSoundEffect *soundBgm;
    QSoundEffect *soundLoss;
    QSoundEffect *soundHurt;
    QSoundEffect *soundFire1;
    QSoundEffect *soundFire2;
    QSoundEffect *soundFire3;

    // 游戏状态
    int hp = 10;
    int score = 0;
    QList<Bullet*> bulletList;
    QList<Chicken*> chickenList;

    int numPerGen = 2;
    Timer *timerGenerate;
    Timer *timerIncreaseNumPerGen;

    QPointF posCrosshair;
    double angleBarrel = 0;
    const QPointF posBattery{640, 600};
    const QPointF posBarrel{592, 585};
    const QPointF centerBarrel{48, 25};

    bool isCoolDown = true;
    bool isFireKeyDown = false;
    Animation animationBarrelFire;

    QFont gameFont;
    bool isQuit = false;
};
#endif // MAINWINDOW_H
