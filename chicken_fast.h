#ifndef _CHICKEN_FAST_H_
#define _CHICKEN_FAST_H_

#include "chicken.h"
#include <QRandomGenerator>

extern Atlas* atlasChickenFast;

class ChickenFast : public Chicken {
public:
    ChickenFast() {
        // 添加动画帧
        animation_run.add_frames(atlasChickenFast->get_textures());
        // 设置移动速度
        speed_run = 80.0f;

        // 设置初始位置 (使用Qt的随机数生成器)
        float randomX = static_cast<float>(QRandomGenerator::global()->bounded(200, 1080));
        float randomY = static_cast<float>(QRandomGenerator::global()->bounded(-100, -50));
        QPointF initialPos(randomX, randomY);
        position = initialPos; // 直接设置position属性

        // 设置动画循环和间隔
        animation_run.set_loop(true);
        animation_run.set_interval(100); // 毫秒
    }

    ~ChickenFast() = default;
};

#endif // _CHICKEN_FAST_H_
