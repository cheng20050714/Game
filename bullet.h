#ifndef _BULLET_H_
#define _BULLET_H_

#include <QPointF>
#include <QRectF>
#include <QPixmap>
#include <QPainter>
#include <QtMath>

// 全局变量存储子弹纹理
extern QPixmap* tex_bullet;

class Bullet {
public:
    Bullet(double angle) {
        this->angle = angle;

        double radians = angle * M_PI / 180.0;
        velocity.setX(qCos(radians) * speed);
        velocity.setY(qSin(radians) * speed); // 速度计算
    }

    ~Bullet() = default;

    void set_position(const QPointF& position) {
        this->position = position;
    }

    const QPointF& get_position() const {
        return position;
    }

    void on_update(float delta) { // 更新
        // 计算新位置
        position.setX(position.x() + velocity.x() * delta);
        position.setY(position.y() + velocity.y() * delta);

        // 检查屏幕边界
        if (position.x() <= 0 || position.x() >= 1280 || position.y() <= 0 || position.y() >= 720) {
            is_valid = false;
        }
    }

    void on_render(QPainter* painter) const {
        if (!is_valid) return;

        // 保存当前画家状态
        painter->save();

        // 平移到子弹位置
        painter->translate(position);

        // 旋转到子弹角度
        painter->rotate(angle);

        // 绘制子弹 (居中对齐)
        QRectF rect_bullet(-4, -2, 8, 4);
        painter->drawPixmap(rect_bullet, *tex_bullet, QRectF(0, 0, tex_bullet->width(), tex_bullet->height()));

        // 恢复画家状态
        painter->restore();
    }

    // 设置子弹失效
    void on_hit() {
        is_valid = false;
    }

    bool can_remove() const {
        return !is_valid;
    }

private:
    double angle = 0;
    QPointF position;
    QPointF velocity;
    bool is_valid = true;
    float speed = 800.0f;
};

#endif // !_BULLET_QT_H_
