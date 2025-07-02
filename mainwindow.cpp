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
    setWindowTitle("VERSION2");
    setFixedSize(1280, 720);
    setCursor(Qt::BlankCursor);


    // 加载资源
    loadResources();


    // 设置生成小鸡的定时器
    // 修改这部分可以修改小鸡生成频率
    timerGenerate->set_one_shot(false);
    timerGenerate->set_wait_time(3.0f);
    timerGenerate->set_on_timeout([this](){
        for (int i = 0; i < numPerGen; i++) {
            int val = QRandomGenerator::global()->bounded(100);
            Chicken* chicken = nullptr;
            if (val < 50)
                chicken = new ChickenSlow();
            else if (val < 80)
                chicken = new ChickenMedium();
            else
                chicken = new ChickenFast();
            chickenList.append(chicken);
        }
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


/*
功能：
    •	加载所有静态图片（背景、子弹、准星、炮台、炮管等）。
    •	使用 Atlas 类加载各种图集资源（动画帧图片）：
    •	炮管开火动画
    •	三种小鸡（Fast、Medium、Slow）
    •	爆炸动画
    •	创建并配置多个 QSoundEffect，用于背景音乐、失败、受伤、开火等音效。
    •	初始化全局图集指针。
    •	加载自定义字体（如果加载失败则回退使用 Arial）。
    •	加载英文单词字典并启动第一次拼写挑战。
 */
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
    // 🆘重大问题🆘 qt不支持mp3 但是修不修都可以 不影响展示
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
    // 替换旧的赋值方式（对象 → 指针）
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

    // 加载单词词典并启动拼写游戏
    loadWordDictionary();
    if (!wordDictionary.isEmpty()) {
        startNewWordChallenge();
    }
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
    // sound_explosion 是全局变量，也需要删除
    delete sound_explosion;
    sound_explosion = nullptr;
}


/*
功能：
    •	读取 :/res/Resources/words_list.txt 中的单词（每两行：英文+中文）。
    •	将其存入 wordDictionary 映射中，供拼写小游戏使用。
    •	成功加载后输出日志。
 */
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

void MainWindow::startNewWordChallenge()
{
    /* Debug内容
    if (wordDictionary.isEmpty()) {
        qDebug() << "词典为空，无法开始拼写挑战";
        return;
    }
    */

    // 随机选择一个单词
    QStringList words = wordDictionary.keys();
    int randomIndex = QRandomGenerator::global()->bounded(words.size());
    currentWord = words[randomIndex];
    currentDefinition = wordDictionary[currentWord];

    playerInput.clear();
    isSpellingGameActive = true;

    qDebug() << "新单词挑战:" << currentWord << "-" << currentDefinition;
}


/*
功能（每帧执行一次）：
    •	更新所有定时器的逻辑。
    •	更新所有子弹的位置与状态。
    •	更新所有小鸡的位置、生命状态及与子弹的碰撞检测。
    •	处理子弹击中、鸡到达底部（玩家受伤）、鸡死亡等逻辑。
    •	自动瞄准：如果有小鸡，则瞄准最靠近底部的一只。
    •	更新炮管开火动画与相机抖动。
    •	如果满足条件则发射子弹。
    •	检查玩家生命值，触发游戏结束。
    •	调用 update() 触发 paintEvent()，完成屏幕重绘。
 */
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

        // 与每个子弹检查碰撞
        for (Bullet* bullet : bulletList) {
            if (!chicken->check_alive() || bullet->can_remove())
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

        if (!chicken->check_alive())
            continue;

        // 如果小鸡到达底部，减少生命值并播放受伤音效
        if (chicken->get_position().y() >= 720) {
            chicken->make_invalid();
            soundHurt->play();
            hp -= 1;
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

    // 如果冷却完毕且按下开火键，发射子弹
    if (isCoolDown && isFireKeyDown) {
        fireWeapon();
    }

    // 添加自动瞄准功能
    if (!chickenList.isEmpty()) {
        // 找到最近的小鸡（以Y坐标最大的为准，也就是最接近底部的）
        Chicken* targetChicken = *std::max_element(chickenList.begin(), chickenList.end(),
                                                   [](Chicken* a, Chicken* b) {
                                                       return a->get_position().y() < b->get_position().y();
                                                   });

        // 计算目标方向
        QPointF targetDirection = targetChicken->get_position() - posBattery;
        angleBarrel = qAtan2(targetDirection.y(), targetDirection.x()) * 180 / M_PI;
    }

    camera->on_update(delta);




    // 检查游戏结束条件
    if (hp <= 0) {
        gameOver();
    }

    // 触发重绘
    update();
}


/*
功能：
    •	使用 QPainter 绘制所有游戏可视元素：
    •	背景图像
    •	所有存活的小鸡
    •	所有子弹
    •	炮台本体（固定位置）
    •	炮管（随目标方向旋转）
    •	炮管静止状态：显示默认图。
    •	炮管开火状态：播放动画帧。
    •	玩家血量（心形图标）
    •	当前分数（右上角文本）
    •	拼写挑战界面：显示释义和玩家输入。
*/
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

            // 调试绘制旋转中心点 (可选)
            painter.setPen(Qt::red);
            painter.drawEllipse(rotationCenter, 5, 5);

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
        // 绘制开火动画 - 使用与炮管相同的旋转逻辑
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

    // 绘制得分 - 使用简化方法
    painter.setFont(gameFont);
    QString strScore = "SCORE: " + QString::number(score);

    // 设置文本位置 - 固定在右上角
    QFontMetrics fm = painter.fontMetrics();
    int textWidth = fm.horizontalAdvance(strScore);
    int textHeight = fm.height();


    // 绘制文本 - 白色，清晰可见
    painter.setPen(Qt::white);
    painter.drawText(width() - textWidth - 20, 15 + textHeight, strScore);

    // 调试 - 输出分数值
    static int lastScore = -1;
    if (score != lastScore) {
        qDebug() << "当前分数:" << score;
        lastScore = score;
    }
    // 绘制拼写游戏界面
    if (isSpellingGameActive) {
        int screenWidth = width();
        int screenHeight = height();

        // 设置更大的字体
        QFont largeFont = gameFont;
        largeFont.setPointSize(40);
        painter.setFont(largeFont);

        // 绘制中文释义
        QFontMetrics fmLarge(largeFont);
        int textWidthDef = fmLarge.horizontalAdvance(currentDefinition);
        int textHeightDef = fmLarge.height();

        // 绘制背景阴影效果
        painter.setPen(QColor(55, 55, 55, 255));
        painter.drawText(
            (screenWidth - textWidthDef) / 2,
            (screenHeight / 2) - 50 + textHeightDef,
            currentDefinition
            );

        // 绘制前景文字
        painter.setPen(QColor(255, 255, 255, 255));
        painter.drawText(
            (screenWidth - textWidthDef) / 2 - 2,
            (screenHeight / 2) - 50 + textHeightDef - 2,
            currentDefinition
            );

        // 绘制玩家输入
        int textWidthInput = fmLarge.horizontalAdvance(playerInput);
        int textHeightInput = fmLarge.height();

        // 绘制背景阴影效果
        painter.setPen(QColor(55, 55, 55, 255));
        painter.drawText(
            (screenWidth - textWidthInput) / 2,
            (screenHeight / 2) + 30 + textHeightInput,
            playerInput
            );

        // 绘制前景文字
        painter.setPen(QColor(255, 255, 255, 255));
        painter.drawText(
            (screenWidth - textWidthInput) / 2 - 2,
            (screenHeight / 2) + 30 + textHeightInput - 2,
            playerInput
            );
    }

    // VERSION2 中不需要准星
    /*
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
    */
}

//VERSION2 中不需要鼠标控制
/*
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    posCrosshair.setX(event->position().x());
    posCrosshair.setY(event->position().y());

    QPointF direction = posCrosshair - posBattery;
    angleBarrel = qAtan2(direction.y(), direction.x()) * 180 / M_PI;

    update();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    isFireKeyDown = true;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    isFireKeyDown = false;
}
*/


/*
键盘输入处理（拼写+退出）：
  ·以下是所有外部设备输入处理
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }

    // 处理拼写游戏的输入
    if (isSpellingGameActive) {
        // 将按键转换为小写字母
        QString keyText = event->text().toLower();

        // 检查是否为字母
        if (keyText.length() == 1 && keyText[0].isLetter()) {
            QChar key = keyText[0];

            // 检查当前输入位置的正确字母
            if (playerInput.length() < currentWord.length() &&
                key == currentWord[playerInput.length()].toLower()) {

                // 添加正确的字母到输入中
                playerInput += key;

                // 每次输入正确字母就发射一枚子弹
                isCoolDown = true; // 确保可以开火
                fireWeapon();

                // 如果完全拼写正确，选择下一个单词
                if (playerInput.toLower() == currentWord.toLower()) {
                    // 加分
                    score += 10;

                    // 延迟一下切换单词，让玩家看到完成
                    QTimer::singleShot(100, this, &MainWindow::startNewWordChallenge);
                }
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    isQuit = true;
    QMainWindow::closeEvent(event);
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


