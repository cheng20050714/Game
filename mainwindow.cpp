#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chicken.h"
#include "chicken_fast.h"
#include "chicken_medium.h"
#include "chicken_slow.h"

#include <QDebug>
#include <QtMath>
#include <QMessageBox>
#include <QPainter>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , camera(new Camera(this))
    , timerGenerate(new Timer(this))
    , timerIncreaseNumPerGen(new Timer(this))
{
    // 这个代码设置了鼠标跟踪功能
    ui->setupUi(this);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    // 设置窗口属性
    setWindowTitle("VERSION3 - 单词匹配游戏");
    setFixedSize(1280, 720);
    setCursor(Qt::BlankCursor);

    // 加载资源
    loadResources();

    // 设置生成小鸡的定时器
    timerGenerate->set_one_shot(false);
    timerGenerate->set_wait_time(3.0f);
    timerGenerate->set_on_timeout([this](){
        // 每次生成一对单词匹配的小鸡
        createWordMatchingPair();
    });
    timerGenerate->start();

    // 设置增加小鸡生成数量的定时器
    timerIncreaseNumPerGen->set_one_shot(false);
    timerIncreaseNumPerGen->set_wait_time(8.0f);
    timerIncreaseNumPerGen->set_on_timeout([this](){
        numPerGen += 0.5;
    });
    timerIncreaseNumPerGen->start();

    // 设置炮管开火动画
    animationBarrelFire.set_loop(false);
    animationBarrelFire.set_interval(40); // 毫秒
    animationBarrelFire.set_center(centerBarrel);
    animationBarrelFire.add_frames(m_atlasBarrelFire.get_textures());
    animationBarrelFire.set_on_finished([this](){
        isCoolDown = true;
    });
    animationBarrelFire.set_position(QPointF(780, 610));

    // 播放背景音乐
    soundBgm->play();

    // 设置游戏循环
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateGame);
    gameTimer->start(1000/144); // 约144fps

    // 设置自动射击计时器
    autoFireTimer = new QTimer(this);
    connect(autoFireTimer, &QTimer::timeout, this, &MainWindow::autoFire);
    autoFireTimer->start(1000); // 每秒尝试自动射击一次

    // 启动帧计时器
    frameTimer.start();
}

MainWindow::~MainWindow()
{
    unloadResources();

    // 清理游戏对象
    for (Bullet* bullet : bulletList) {
        delete bullet;
    }
    bulletList.clear();

    for (Chicken* chicken : chickenList) {
        delete chicken;
    }
    chickenList.clear();

    delete ui;
}

void MainWindow::loadResources()
{
    // 加载图片
    texHeart.load(":/res/Resources/heart.png");
    texBullet.load(":/res/Resources/bullet.png");
    tex_bullet = &texBullet;
    texBattery.load(":/res/Resources/battery.png");
    texCrosshair.load(":/res/Resources/crosshair.png");
    texBackground.load(":/res/Resources/background.png");
    texBarrelIdle.load(":/res/Resources/barrel_idle.png");

    // 加载图集
    m_atlasBarrelFire.load(":/res/Resources/barrel_fire_%1.png", 3);
    m_atlasChickenFast.load(":/res/Resources/chiikawa_3/chiikawa_3-%1.png", 6);
    m_atlasChickenMedium.load(":/res/Resources/chiikawa_1/chiikawa_1-%1.png", 17);
    m_atlasChickenSlow.load(":/res/Resources/chiikawa_2/chiikawa_2-%1.png", 21);
    m_atlasExplosion.load(":/res/Resources/explosion_%1.png", 5);

    // 加载音频
    soundBgm = new QSoundEffect(this);
    soundBgm->setSource(QUrl("qrc:/res/Resources/bgm.mp3"));
    soundBgm->setLoopCount(QSoundEffect::Infinite);

    soundLoss = new QSoundEffect(this);
    soundLoss->setSource(QUrl("qrc:/res/Resources/loss.mp3"));

    soundHurt = new QSoundEffect(this);
    soundHurt->setSource(QUrl("qrc:/res/Resources/hurt.wav"));

    soundFire1 = new QSoundEffect(this);
    soundFire1->setSource(QUrl("qrc:/res/Resources/fire_1.wav"));

    soundFire2 = new QSoundEffect(this);
    soundFire2->setSource(QUrl("qrc:/res/Resources/fire_2.wav"));

    soundFire3 = new QSoundEffect(this);
    soundFire3->setSource(QUrl("qrc:/res/Resources/fire_3.wav"));

    sound_explosion = new QSoundEffect(this);
    sound_explosion->setSource(QUrl("qrc:/res/Resources/explosion.wav"));


    // 初始化全局变量
    atlasChickenFast = new Atlas(m_atlasChickenFast);
    atlasChickenMedium = new Atlas(m_atlasChickenMedium);
    atlasChickenSlow = new Atlas(m_atlasChickenSlow);
    atlasExplosion = new Atlas(m_atlasExplosion);

    // 加载字体
    int fontId = QFontDatabase::addApplicationFont(":/res/Resources/IPix.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            gameFont.setFamily(fontFamilies.at(0));
        } else {
            gameFont.setFamily("Arial");
        }
    } else {
        gameFont.setFamily("Arial");
    }
    gameFont.setPointSize(28);

    // 加载单词词典
    loadWordDictionary();
}

void MainWindow::unloadResources()
{
    // Qt自动管理QPixmap，但需要删除QSoundEffect对象
    delete soundBgm;
    delete soundLoss;
    delete soundHurt;
    delete soundFire1;
    delete soundFire2;
    delete soundFire3;
    delete sound_explosion;
    sound_explosion = nullptr;
}

// 添加创建单词匹配对的函数
void MainWindow::createWordMatchingPair()
{
    // 确保字典有单词
    if (wordDictionary.isEmpty()) {
        return;
    }

    // 随机选择一个单词
    QStringList words = wordDictionary.keys();
    int randomIndex = QRandomGenerator::global()->bounded(words.size());
    QString englishWord = words[randomIndex];
    QString chineseWord = wordDictionary[englishWord];

    // 创建两只小鸡，一个显示英文，一个显示中文
    // 随机决定小鸡类型
    for (int i = 0; i < 2; i++) {
        int val = QRandomGenerator::global()->bounded(100);
        Chicken* chicken = nullptr;

        if (val < 50)
            chicken = new ChickenSlow();
        else if (val < 80)
            chicken = new ChickenMedium();
        else
            chicken = new ChickenFast();

        // 设置单词和语言标记
        if (i == 0) {
            chicken->set_word(englishWord);
            chicken->set_is_english(true);
        } else {
            chicken->set_word(chineseWord);
            chicken->set_is_english(false);
        }

        // 添加到列表
        chickenList.append(chicken);
    }

    // 设置互为配对
    chickenList[chickenList.size()-2]->set_pair(chickenList[chickenList.size()-1]);
    chickenList[chickenList.size()-1]->set_pair(chickenList[chickenList.size()-2]);
}

void MainWindow::loadWordDictionary()
{
    // 从资源文件中加载单词列表
    QFile file(":/res/Resources/words_list.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString english, chinese;
        while (!in.atEnd()) {
            english = in.readLine();
            if (!in.atEnd()) {
                chinese = in.readLine();
                wordDictionary[english] = chinese;
            }
        }
        file.close();
        qDebug() << "加载了" << wordDictionary.size() << "个单词";
    } else {
        qDebug() << "无法打开单词列表文件";
    }
}

void MainWindow::updateGame()
{
    static int frameCounter = 0;
    if (frameCounter++ % 60 == 0) {
        qDebug() << "当前小鸡数量:" << chickenList.size();
    }

    // 计算帧时间
    float delta = frameTimer.elapsed() / 1000.0f;
    frameTimer.restart();

    // 如果游戏结束，不再更新
    if (isQuit) {
        return;
    }

    // 更新定时器
    timerGenerate->on_update(delta);
    timerIncreaseNumPerGen->on_update(delta);

    // 更新子弹
    for (Bullet* bullet : bulletList) {
        bullet->on_update(delta);
    }

    // 更新小鸡并检查碰撞
    for (Chicken* chicken : chickenList) {
        chicken->on_update(delta);

        // 小鸡到达底部，玩家受伤
        if (chicken->check_alive() && chicken->get_position().y() > height() - 50) {
            chicken->make_invalid();
            hp -= 1;
            soundHurt->play();
        }

        // 与每个子弹检查碰撞
        for (Bullet* bullet : bulletList) {
            if (!chicken->check_alive() || bullet->can_remove() || !chicken->can_be_shot_by_bullet())
                continue;

            const QPointF& posBullet = bullet->get_position();
            const QPointF& posChicken = chicken->get_position();
            static const QSizeF sizeChicken(30, 40);

            if (posBullet.x() >= posChicken.x() - sizeChicken.width() / 2
                && posBullet.x() <= posChicken.x() + sizeChicken.width() / 2
                && posBullet.y() >= posChicken.y() - sizeChicken.height() / 2
                && posBullet.y() <= posChicken.y() + sizeChicken.height() / 2) {
                score += 1;
                bullet->on_hit();
                chicken->on_hurt(); // is_alive: false
            }
        }
    }

    // 移除无效的子弹
    for (auto it = bulletList.begin(); it != bulletList.end();) {
        if ((*it)->can_remove()) {
            delete *it;
            it = bulletList.erase(it);
        } else {
            ++it;
        }
    }

    // 移除无效的小鸡
    for (auto it = chickenList.begin(); it != chickenList.end();) {
        if ((*it)->can_remove()) {
            delete *it;
            it = chickenList.erase(it);
        } else {
            ++it;
        }
    }

    // 按Y坐标排序小鸡，使远处的先绘制
    std::sort(chickenList.begin(), chickenList.end(),
              [](const Chicken* chicken1, const Chicken* chicken2) {
                  return chicken1->get_position().y() < chicken2->get_position().y();
              });

    // 处理开火相关逻辑
    if (!isCoolDown) {
        camera->shake(3.0f, 0.1f);
        animationBarrelFire.on_update(delta);
    }

    camera->on_update(delta);

    // 检查游戏结束条件
    if (hp <= 0) {
        gameOver();
    }

    // 触发重绘
    update();
}

// 自动射击函数
void MainWindow::autoFire()
{
    if (isQuit || !isCoolDown) {
        return;
    }

    // 查找可射击的小鸡
    Chicken* targetChicken = nullptr;
    for (Chicken* chicken : chickenList) {
        if (chicken->check_alive() && chicken->can_be_shot_by_bullet()) {
            targetChicken = chicken;
            break;
        }
    }

    if (targetChicken) {
        // 计算目标方向
        QPointF targetDirection = targetChicken->get_position() - posBattery;
        angleBarrel = qAtan2(targetDirection.y(), targetDirection.x()) * 180 / M_PI;

        // 发射子弹
        fireWeapon();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    //相机抖动
    QPointF cameraPos = camera->get_position();
    painter.translate(-cameraPos.x(), -cameraPos.y());

    // 绘制背景图
    if (!texBackground.isNull()) {
        int widthBg = texBackground.width();
        int heightBg = texBackground.height();
        QRectF rectBackground(
            (width() - widthBg) / 2.0f,
            (height() - heightBg) / 2.0f,
            widthBg,
            heightBg
            );
        painter.drawPixmap(rectBackground.toRect(), texBackground);
    }

    // 绘制小鸡
    for (Chicken* chicken : chickenList) {
        chicken->on_render(&painter);
    }

    // 绘制子弹
    for (Bullet* bullet : bulletList) {
        bullet->on_render(&painter);
    }

    // 绘制炮台
    if (!texBattery.isNull()) {
        int widthBattery = texBattery.width();
        int heightBattery = texBattery.height();
        QRectF rectBattery(
            posBattery.x() - widthBattery / 2.0f,
            posBattery.y() - heightBattery / 2.0f,
            widthBattery,
            heightBattery
            );
        painter.drawPixmap(rectBattery.toRect(), texBattery);
    }

    // 绘制炮管
    if (isCoolDown) {
        if (!texBarrelIdle.isNull()) {
            painter.save();

            // 设置旋转中心点
            QPointF rotationCenter = posBarrel + centerBarrel;

            // 应用旋转变换
            painter.translate(rotationCenter);
            painter.rotate(angleBarrel);
            painter.translate(-centerBarrel);

            // 绘制炮管
            painter.drawPixmap(0, 0, texBarrelIdle);

            // 恢复原始变换
            painter.restore();
        }
    } else {
        // 绘制开火动画
        const auto& textures = m_atlasBarrelFire.get_textures();
        if (!textures.empty()) {
            painter.save();

            // 使用与静止炮管相同的旋转中心和变换
            QPointF rotationCenter = posBarrel + centerBarrel;

            // 应用相同的旋转变换
            painter.translate(rotationCenter);
            painter.rotate(angleBarrel);
            painter.translate(-centerBarrel);

            // 静态变量来模拟动画帧
            static int frameIndex = 0;
            static QElapsedTimer frameTimer;

            // 每50毫秒更换一帧
            if (frameTimer.isValid() && frameTimer.elapsed() > 50) {
                frameIndex = (frameIndex + 1) % textures.size();
                frameTimer.restart();
            } else if (!frameTimer.isValid()) {
                frameTimer.start();
            }

            // 在相同位置绘制动画帧
            painter.drawPixmap(0, 0, textures[frameIndex]);

            painter.restore();
        } else {
            // 如果没有纹理，使用静止炮管纹理
            painter.save();

            QPointF rotationCenter = posBarrel + centerBarrel;
            painter.translate(rotationCenter);
            painter.rotate(angleBarrel);
            painter.translate(-centerBarrel);
            painter.drawPixmap(0, 0, texBarrelIdle);

            painter.restore();
        }
    }

    // 绘制生命值
    if (!texHeart.isNull()) {
        int widthHeart = texHeart.width();
        int heightHeart = texHeart.height();
        for (int i = 0; i < hp; i++) {
            QRect rectDst(
                15 + (widthHeart + 10) * i,
                15,
                widthHeart,
                heightHeart
                );
            painter.drawPixmap(rectDst, texHeart);
        }
    }

    // 绘制得分
    painter.setFont(gameFont);
    QString strScore = "SCORE: " + QString::number(score);

    // 设置文本位置
    QFontMetrics fm = painter.fontMetrics();
    int textWidth = fm.horizontalAdvance(strScore);
    int textHeight = fm.height();

    // 绘制文本
    painter.setPen(Qt::white);
    painter.drawText(width() - textWidth - 20, 15 + textHeight, strScore);

    // 绘制连线
    drawConnectionLine(&painter);

    // 绘制准星
    if (!texCrosshair.isNull()) {
        int widthCrosshair = texCrosshair.width();
        int heightCrosshair = texCrosshair.height();
        QRectF rectCrosshair(
            posCrosshair.x() - widthCrosshair / 2.0f,
            posCrosshair.y() - heightCrosshair / 2.0f,
            widthCrosshair,
            heightCrosshair
            );
        painter.drawPixmap(rectCrosshair.toRect(), texCrosshair);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (isQuit) return;

    // 处理小鸡点击
    Chicken* clickedChicken = getClickedChicken(event->pos());
    if (clickedChicken) {
        processChickenClick(clickedChicken);
    } else {
        // 点击空白处，重置选择状态
        if (firstSelectedChicken) {
            firstSelectedChicken->set_selected(false);
            firstSelectedChicken = nullptr;
        }
    }

    // 触发重绘
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // 仅处理鼠标释放事件，不再控制开火
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    posCrosshair.setX(event->position().x());
    posCrosshair.setY(event->position().y());
    update();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    isQuit = true;
    QMainWindow::closeEvent(event);
}

Chicken* MainWindow::getClickedChicken(const QPoint& clickPos)
{
    for (Chicken* chicken : chickenList) {
        if (!chicken->check_alive())
            continue;

        QRect buttonRect = chicken->get_word_button_rect();
        if (buttonRect.contains(clickPos)) {
            return chicken;
        }
    }
    return nullptr;
}

void MainWindow::processChickenClick(Chicken* chicken)
{
    // 如果已经匹配了，忽略点击
    if (chicken->is_matched_word())
        return;

    // 第一次点击
    if (!firstSelectedChicken) {
        chicken->set_selected(true);
        firstSelectedChicken = chicken;
    }
    // 第二次点击
    else {
        // 不能点击同一个小鸡
        if (chicken == firstSelectedChicken)
            return;

        // 检查是否匹配
        if (chicken->is_pair_with(firstSelectedChicken)) {
            // 匹配成功
            chicken->set_matched(true);
            firstSelectedChicken->set_matched(true);

            // 创建连线计时器
            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);

            // 创建连线并添加到活跃连线列表
            activeConnections.append(
                ChickenConnection(chicken, firstSelectedChicken, timer)
                );

            // 设置计时器结束时移除连线
            connect(timer, &QTimer::timeout, this, [this, timer]() {
                // 在计时器结束时移除对应的连线
                for (int i = 0; i < activeConnections.size(); ++i) {
                    if (activeConnections[i].timer == timer) {
                        activeConnections.removeAt(i);
                        break;
                    }
                }
                update();
            });

            // 启动计时器 - 连线显示3秒
            timer->start(3000);

            // 额外奖励分数
            score += 5;
        }

        // 重置第一个选择
        firstSelectedChicken->set_selected(false);
        firstSelectedChicken = nullptr;
    }
}





void MainWindow::drawConnectionLine(QPainter* painter)
{
    painter->save();
    painter->setPen(QPen(Qt::green, 3, Qt::DashLine));

    // 绘制所有活跃的连线
    for (const auto& connection : activeConnections) {
        // 只有当两个小鸡都还活着时才绘制连线
        if (connection.chicken1->check_alive() && connection.chicken2->check_alive()) {
            QPoint start = connection.chicken1->get_word_button_rect().center();
            QPoint end = connection.chicken2->get_word_button_rect().center();
            painter->drawLine(start, end);
        }
    }

    painter->restore();
}

void MainWindow::fireWeapon()
{
    // 重置动画和冷却状态
    animationBarrelFire.reset();
    isCoolDown = false;

    // 创建子弹
    Bullet* bullet = new Bullet(angleBarrel);

    // 计算子弹位置
    const float lengthBarrel = 105;
    const QPointF posBarrelCenter(640, 610);

    double angleBullet = angleBarrel; // 移除随机偏差，使用精确角度
    double radians = angleBullet * M_PI / 180;
    QPointF direction(qCos(radians), qSin(radians));
    QPointF bulletPos = posBarrelCenter + direction * lengthBarrel;

    bullet->set_position(bulletPos);
    bulletList.append(bullet);

    // 播放随机开火音效
    int soundIndex = QRandomGenerator::global()->bounded(3);
    switch (soundIndex) {
    case 0: soundFire1->play(); break;
    case 1: soundFire2->play(); break;
    case 2: soundFire3->play(); break;
    }
}

void MainWindow::gameOver()
{
    isQuit = true;

    // 停止背景音乐，播放失败音乐
    soundBgm->stop();
    soundLoss->play();

    // 显示游戏结束消息
    QString msg = "Final score: " + QString::number(score);
    QMessageBox::information(this, "Game Over!", msg);

    // 游戏结束后关闭窗口
    QTimer::singleShot(3000, this, &MainWindow::close);
}
