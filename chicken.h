#ifndef _CHICKEN_H_
#define _CHICKEN_H_

#include "animation.h"
#include "atlas.h"

#include <QPointF>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMultimedia/QSoundEffect>
#include <QString>
#include <QRect>

// 全局资源声明
extern Atlas* atlasExplosion;
extern QSoundEffect* sound_explosion; // 爆炸音效

class Chicken : public QObject {
    Q_OBJECT

public:
    Chicken(QObject* parent = nullptr) : QObject(parent) {
        // 设置奔跑动画
        animation_run.set_loop(true);
        animation_run.set_interval(100); // 毫秒为单位

        // 设置爆炸动画
        animation_explosive.set_loop(false);
        animation_explosive.set_interval(80);
        animation_explosive.add_frames(atlasExplosion->get_textures()); // 添加爆炸动画帧
        animation_explosive.set_on_finished([this]() {
            is_valid = false;
        });

        // 随机生成水平位置
        position.setX(40.0f + QRandomGenerator::global()->bounded(1200));
        position.setY(-50);

        // 设置初始动画
        animation_current = &animation_run;
    }

    ~Chicken() = default;

    const QPointF& get_position() const {
        return position;
    }

    void on_update(float delta) {
        if (is_valid) {
            // 更新位置
            position.setY(position.y() + speed_run * delta);
        }

        // 更新动画状态
        animation_current = (is_alive ? &animation_run : &animation_explosive);
        animation_current->set_position(position);
        animation_current->on_update(delta);
    }

    void on_render(QPainter* painter) const {
        animation_current->render(painter);

        // 绘制单词按钮
        if (is_alive && !word.isEmpty()) {
            QRect buttonRect = get_word_button_rect();

            // 绘制按钮背景
            painter->setBrush(is_selected ? Qt::yellow : Qt::white);
            painter->setPen(Qt::black);
            painter->drawRect(buttonRect);

            // 绘制单词文本
            painter->setFont(QFont("Arial", 12, QFont::Bold));
            painter->drawText(buttonRect, Qt::AlignCenter, word);

            // 如果匹配了，绘制一个绿色边框
            if (is_matched) {
                painter->setPen(QPen(Qt::green, 3));
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(buttonRect);
            }
        }
    }

    void on_hurt() {
        is_alive = false;

        animation_explosive.reset();
        // 播放爆炸音效
        if (sound_explosion) {
            sound_explosion->play();
        }
    }

    void make_invalid() {
        is_valid = false;
    }

    bool check_alive() const {
        return is_alive;
    }

    bool can_remove() const {
        return !is_valid;
    }

    // 新增方法 - 单词相关
    void set_word(const QString& w) {
        word = w;
    }

    QString get_word() const {
        return word;
    }

    void set_is_english(bool value) {
        is_english = value;
    }

    bool get_is_english() const {
        return is_english;
    }

    void set_matched(bool matched) {
        is_matched = matched;
        // 匹配后才能被射击
        can_be_shot = matched;
    }

    bool is_matched_word() const {
        return is_matched;
    }

    void set_pair(Chicken* other) {
        pair_chicken = other;
    }

    Chicken* get_pair() const {
        return pair_chicken;
    }

    bool is_pair_with(Chicken* other) const {
        return pair_chicken == other;
    }

    // 获取单词按钮区域
    QRect get_word_button_rect() const {
        int width = 120;
        int height = 40;
        return QRect(position.x() - width/2, position.y() - height/2 - 40, width, height);
    }

    // 选择状态
    void set_selected(bool selected) {
        is_selected = selected;
    }

    bool get_selected() const {
        return is_selected;
    }

    // 判断是否可以被射击
    bool can_be_shot_by_bullet() const {
        return can_be_shot;
    }

protected:
    float speed_run = 10.0f;
    Animation animation_run;

protected:
    QPointF position;
    Animation animation_explosive;
    Animation* animation_current = nullptr;

    bool is_alive = true;
    bool is_valid = true;

    // 单词相关属性
    QString word;                // 显示的单词
    bool is_english = true;      // 是否为英文
    bool is_matched = false;     // 是否已匹配
    Chicken* pair_chicken = nullptr; // 配对的小鸡
    bool is_selected = false;    // 是否被选中
    bool can_be_shot = false;    // 是否可以被子弹射击
};

#endif // !_CHICKEN_QT_H_
