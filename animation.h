#ifndef _ANIMATION_QT_H_
#define _ANIMATION_QT_H_

#include "camera.h"
#include "atlas.h"

#include <QPixmap>
#include <QTimer>
#include <QRectF>
#include <QPointF>
#include <QTransform>
#include <QPainter>
#include <vector>
#include <functional>

class Animation : public QObject {
    Q_OBJECT

public:
    Animation(QObject* parent = nullptr) : QObject(parent) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() {
            idx_frame++;
            if (idx_frame >= frame_list.size()) {
                idx_frame = is_loop ? 0 : frame_list.size() - 1;
                if (!is_loop && on_finished) {
                    on_finished();
                }
            }
        });
        //timer->start();
    }

    ~Animation() = default;

    void reset() {
        timer->start();
        idx_frame = 0;
    }

    void set_position(const QPointF& position) {
        this->position = position;
    }

    void set_rotation(double angle) {
        this->angle = angle;
    }

    void set_center(const QPointF& center) {
        this->center = center;
    }

    void set_loop(bool is_loop) {
        this->is_loop = is_loop;
    }

    void set_interval(int interval) {
        timer->setInterval(interval);
        // 只有在设置了间隔后才启动定时器
        if (!timer->isActive() && !frame_list.empty()) {
            timer->start();
        }
    }

    void set_on_finished(std::function<void()> on_finished) {
        this->on_finished = on_finished;
    }

    // 添加 on_update 方法，用于手动更新动画状态
    void on_update(float delta) {
        // Qt的计时器是自动更新的，所以这个方法在大多数情况下不需要做任何事情
        // 但为了兼容旧代码，我们保留这个方法
        // 如果需要，可以在这里添加额外的更新逻辑
    }

    // 从水平切分的sprite sheet添加帧
    void add_frame(const QPixmap& texture, int num_h) {
        int width = texture.width();
        int height = texture.height();
        int width_frame = width / num_h;

        for (int i = 0; i < num_h; i++) {
            QRect rect_src(i * width_frame, 0, width_frame, height);
            frame_list.emplace_back(texture, rect_src);
        }
    }

    // 添加Atlas中的所有帧
    void add_frame(const Atlas* atlas) {
        for (int i = 0; i < atlas->get_size(); i++) {
            QPixmap texture = atlas->get_texture(i);
            QRect rect_src(0, 0, texture.width(), texture.height());
            frame_list.emplace_back(texture, rect_src);
        }
    }

    // 从图集添加帧
    void add_frames(const std::vector<QPixmap>& textures) {
        for (const auto& texture : textures) {
            QRect rect_src(0, 0, texture.width(), texture.height());
            frame_list.emplace_back(texture, rect_src);
        }
    }

    // 渲染当前帧到QPainter
    void render(QPainter* painter) const {
        if (frame_list.empty() || idx_frame >= frame_list.size()) return;

        const Frame& frame = frame_list[idx_frame];

        QRectF rect_dst(
            position.x() - frame.rect_src.width() / 2.0,
            position.y() - frame.rect_src.height() / 2.0,
            frame.rect_src.width(),
            frame.rect_src.height()
            );

        // 保存当前变换
        painter->save();

        // 应用旋转
        if (angle != 0) {
            QPointF rotationCenter;
            if (center.isNull()) {
                rotationCenter = rect_dst.center();
            } else {
                rotationCenter = QPointF(position.x() + center.x(), position.y() + center.y());
            }

            painter->translate(rotationCenter);
            painter->rotate(angle);
            painter->translate(-rotationCenter);
        }

        // 绘制图像
        painter->drawPixmap(rect_dst, frame.texture, frame.rect_src);

        // 恢复变换
        painter->restore();
    }

    // 在Camera的坐标系中渲染
    void on_render(const Camera& camera) const {
        // 这个方法用于兼容旧代码，实际上委托给Camera类处理渲染
        // 具体实现取决于您的Camera类如何转换为Qt
    }

private:
    struct Frame {
        QRect rect_src;
        QPixmap texture;

        Frame() = default;
        Frame(const QPixmap& texture, const QRect& rect_src)
            : texture(texture), rect_src(rect_src) {}
        ~Frame() = default;
    };

private:
    QPointF position;
    double angle = 0;
    QPointF center;

    QTimer* timer;
    bool is_loop = true;
    size_t idx_frame = 0;
    std::vector<Frame> frame_list;
    std::function<void()> on_finished;
};

#endif // !_ANIMATION_QT_H_
