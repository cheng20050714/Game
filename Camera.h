#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <QTimer>
#include <QPointF>
#include <QPainter>
#include <QObject>
#include <QRandomGenerator>
#include <functional>

class Camera : public QObject {
    Q_OBJECT

public:
    // 在绘制区域上绘制，向量都为相对位置
    Camera(QObject* parent = nullptr) : QObject(parent) {
        timer_shaker = new QTimer(this);
        timer_shaker->setSingleShot(true);

        connect(timer_shaker, &QTimer::timeout, this, [this]() {
            is_shaking = false;
            reset();
        });
    }

    ~Camera() = default;

    const QPointF& get_position() const {
        return position;
    }

    void reset() {
        position.setX(0);
        position.setY(0);
    }

    void on_update(float delta) {
        // Qt的计时器是自动更新的，不需要手动调用on_update

        if (is_shaking) {
            // 使用Qt的随机数生成器
            QRandomGenerator* rand = QRandomGenerator::global();
            position.setX((rand->bounded(100) - 50) / 50.0f * shaking_strength);
            position.setY((rand->bounded(100) - 50) / 50.0f * shaking_strength);
        }
    }

    // strength: 震动强度
    // duration: 持续时间 (毫秒)
    void shake(float strength, float duration) {
        is_shaking = true;
        shaking_strength = strength;
        timer_shaker->setInterval(int(duration * 1000)); // 转换为毫秒
        timer_shaker->start();
    }

    // 应用相机变换到QPainter
    void apply_to_painter(QPainter* painter) const {
        // 移动画布以应用相机位置偏移
        painter->translate(-position.x(), -position.y());
    }

    // 将纹理渲染到指定位置，考虑相机偏移
    void render_texture(QPainter* painter, const QPixmap& texture,
                        const QRectF& rect_dst, double angle = 0,
                        const QPointF& center = QPointF()) const {
        // 保存当前画家状态
        painter->save();

        // 应用相机变换
        QRectF rect_dst_win = rect_dst;
        rect_dst_win.translate(-position);

        // 如果需要旋转
        if (angle != 0) {
            // 计算旋转中心
            QPointF rotationCenter;
            if (center.isNull()) {
                rotationCenter = rect_dst_win.center();
            } else {
                rotationCenter = rect_dst_win.topLeft() + center;
            }

            // 移动到旋转中心，旋转，再移回来
            painter->translate(rotationCenter);
            painter->rotate(angle);
            painter->translate(-rotationCenter);
        }

        // 绘制纹理
        painter->drawPixmap(rect_dst_win, texture, QRectF(0, 0, texture.width(), texture.height()));

        // 恢复画家状态
        painter->restore();
    }

    // 重载版本，支持源矩形
    void render_texture(QPainter* painter, const QPixmap& texture,
                        const QRect& rect_src, const QRectF& rect_dst,
                        double angle = 0, const QPointF& center = QPointF()) const {
        // 保存当前画家状态
        painter->save();

        // 应用相机变换
        QRectF rect_dst_win = rect_dst;
        rect_dst_win.translate(-position);

        // 如果需要旋转
        if (angle != 0) {
            // 计算旋转中心
            QPointF rotationCenter;
            if (center.isNull()) {
                rotationCenter = rect_dst_win.center();
            } else {
                rotationCenter = rect_dst_win.topLeft() + center;
            }

            // 移动到旋转中心，旋转，再移回来
            painter->translate(rotationCenter);
            painter->rotate(angle);
            painter->translate(-rotationCenter);
        }

        // 绘制纹理（指定源矩形）
        painter->drawPixmap(rect_dst_win, texture, rect_src);

        // 恢复画家状态
        painter->restore();
    }

private:
    QPointF position;
    QTimer* timer_shaker;
    bool is_shaking = false;
    float shaking_strength = 0;
};

#endif // !_CAMERA_QT_H_
