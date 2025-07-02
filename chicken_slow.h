#ifndef _CHICKEN_SLOW_H_
#define _CHICKEN_SLOW_H_

#include "chicken.h"
#include <QRandomGenerator>

extern Atlas* atlasChickenSlow;

class ChickenSlow : public Chicken {
public:
    ChickenSlow() {
        // 添加动画帧
        animation_run.add_frames(atlasChickenSlow->get_textures());
        // 设置移动速度
        speed_run = 30.0f;

        // 设置初始位置 (使用Qt的随机数生成器)
        float randomX = static_cast<float>(QRandomGenerator::global()->bounded(200, 1080));
        float randomY = static_cast<float>(QRandomGenerator::global()->bounded(-100, -50));
        QPointF initialPos(randomX, randomY);
        position = initialPos; // 直接设置position属性，而不是调用set_position

        // 设置动画循环和间隔
        animation_run.set_loop(true);
        animation_run.set_interval(150); // 毫秒
    }

    ~ChickenSlow() = default;
};

#endif // _CHICKEN_SLOW_H_
