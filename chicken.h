#ifndef _CHICKEN_H_
#define _CHICKEN_H_

#include "animation.h"
#include "atlas.h"

#include <QPointF>
#include <QPainter>
#include <QRandomGenerator>
#include <QtMultimedia/QSoundEffect>

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

protected:
    float speed_run = 10.0f;
    Animation animation_run;

protected:
    QPointF position;
    Animation animation_explosive;
    Animation* animation_current = nullptr;

    bool is_alive = true;
    bool is_valid = true;
};

#endif // !_CHICKEN_QT_H_
