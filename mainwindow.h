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



// ...existing code...
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    //void mouseMoveEvent(QMouseEvent *event) override;
    //void mousePressEvent(QMouseEvent *event) override;
    //void mouseReleaseEvent(QMouseEvent *event) override;
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

    // 新增函数
    void loadWordDictionary();
    void startNewWordChallenge();

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

    Atlas m_atlasBarrelFire;
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

    // 拼写游戏相关 VERISON2
    QMap<QString, QString> wordDictionary; // 英文单词到中文释义的映射
    QString currentWord;                   // 当前要拼写的单词
    QString currentDefinition;             // 当前单词的中文释义
    QString playerInput;                   // 玩家当前输入
    bool isSpellingGameActive = false;     // 拼写游戏是否激活

    QFont gameFont;
    bool isQuit = false;
};
#endif // MAINWINDOW_H
