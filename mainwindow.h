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
    void autoFire();

    // 单词相关功能
    void loadWordDictionary();
    void startNewWordChallenge();

    // 创建单词匹配的小鸡对
    void createWordMatchingPair();

    // 处理小鸡点击事件
    Chicken* getClickedChicken(const QPoint& clickPos);
    void processChickenClick(Chicken* chicken);

    // 绘制连线
    void drawConnectionLine(QPainter* painter);
    // 显示连线一段时间后消失
    void startConnectionLineTimer();

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

    // 拼写游戏相关
    QMap<QString, QString> wordDictionary; // 英文单词到中文释义的映射
    QString currentWord;                   // 当前要拼写的单词
    QString currentDefinition;             // 当前单词的中文释义
    QString playerInput;                   // 玩家当前输入
    bool isSpellingGameActive = false;     // 拼写游戏是否激活

    // 单词匹配相关 - 新增
    Chicken* firstSelectedChicken = nullptr; // 第一个选中的小鸡
    //QPointF connectionStart;                 // 连线起点
    //QPointF connectionEnd;                   // 连线终点
    //bool isConnectionActive = false;         // 是否显示连线
    //QTimer* connectionDisplayTimer;          // 连线显示计时器
    QTimer *autoFireTimer;

    QFont gameFont;
    bool isQuit = false;

    // 在类定义中添加:
    struct ChickenConnection {
        Chicken* chicken1;
        Chicken* chicken2;
        QTimer* timer;

        ChickenConnection(Chicken* c1, Chicken* c2, QTimer* t) :
            chicken1(c1), chicken2(c2), timer(t) {}
    };
    QList<ChickenConnection> activeConnections; // 所有活跃的连线
};
#endif // MAINWINDOW_H
