#ifndef _TIMER_H_
#define _TIMER_H_

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>

class Timer : public QObject {
    Q_OBJECT

public:
    Timer(QObject* parent = nullptr) : QObject(parent) {
        // 创建 Qt 计时器
        qtTimer = new QTimer(this);
        // 连接超时信号到处理槽
        connect(qtTimer, &QTimer::timeout, this, &Timer::handleTimeout);
    }

    ~Timer() = default;

    void restart() {
        // 重置状态
        shotted = false;
        // 如果计时器正在运行，重新启动它
        if (qtTimer->isActive()) {
            qtTimer->start();
        }
    }

    void set_wait_time(float val) {
        // Qt计时器使用整数毫秒，转换浮点秒为毫秒
        wait_time = val;
        qtTimer->setInterval(static_cast<int>(val * 1000));
    }

    // 设置是否定时器触发单次/循环触发
    void set_one_shot(bool flag) {
        one_shot = flag;
        qtTimer->setSingleShot(flag);
    }

    void set_on_timeout(std::function<void()> on_timeout) {
        this->on_timeout = on_timeout;
    }

    void pause() {
        if (qtTimer->isActive()) {
            // 停止计时器但记住它是暂停状态
            qtTimer->stop();
            paused = true;
        }
    }

    void resume() {
        if (paused) {
            // 恢复计时器
            qtTimer->start();
            paused = false;
        }
    }

    // 与原始版本兼容的更新方法
    // 在Qt中，计时器是自动更新的，但保留此方法以兼容现有代码
    void on_update(float delta) {
        // 在Qt实现中，这个方法实际上什么也不做
        // 因为QTimer已经处理了计时
    }

    // 手动启动计时器
    void start() {
        paused = false;
        shotted = false;
        qtTimer->start();
    }

    // 手动停止计时器
    void stop() {
        qtTimer->stop();
        paused = true;
    }

    // 检查计时器是否正在运行
    bool is_active() const {
        return qtTimer->isActive();
    }

private slots:
    void handleTimeout() {
        shotted = true;
        if (on_timeout) {
            on_timeout();
        }

        // 对于非单次定时器，Qt会自动重复
        // 所以不需要额外处理
    }

private:
    float wait_time = 1.0f; // 存储以秒为单位的等待时间
    bool shotted = false;   // 定时器是否已经触发过事件
    bool one_shot = false;  // 是否是单次触发
    bool paused = false;    // 是否暂停
    QTimer* qtTimer;        // Qt的定时器对象
    std::function<void()> on_timeout; // 超时回调函数
};

#endif // !_TIMER_QT_H_
