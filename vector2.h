#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <QtMath>
#include <QPointF>
#include <QVector2D>

class Vector2 {
public:
    float x = 0;
    float y = 0;

public:
    Vector2() = default;
    ~Vector2() = default;

    Vector2(float x, float y) : x(x), y(y) {}

    // Qt 互操作构造函数
    explicit Vector2(const QPointF& point) : x(point.x()), y(point.y()) {}
    explicit Vector2(const QVector2D& vec) : x(vec.x()), y(vec.y()) {}

    Vector2 operator+(const Vector2& vec) const {
        return Vector2(x + vec.x, y + vec.y);
    }

    void operator+=(const Vector2& vec) {
        x += vec.x, y += vec.y;
    }

    void operator-=(const Vector2& vec) {
        x -= vec.x, y -= vec.y;
    }

    Vector2 operator-(const Vector2& vec) const {
        return Vector2(x - vec.x, y - vec.y);
    }

    float operator*(const Vector2& vec) const {
        return x * vec.x + y * vec.y;
    }

    Vector2 operator*(float val) const {
        return Vector2(x * val, y * val);
    }

    void operator*=(float val) {
        x *= val, y *= val;
    }

    Vector2 operator/(float val) const {
        return Vector2(x / val, y / val);
    }

    void operator/=(float val) {
        x /= val, y /= val;
    }

    float length() const {
        return qSqrt(x * x + y * y);
    }

    Vector2 normalize() const {
        float len = length();
        if (qFuzzyIsNull(len)) {
            return Vector2(0, 0);
        }
        return Vector2(x / len, y / len);
    }

    // 转换为 Qt 类型
    QPointF toPointF() const {
        return QPointF(x, y);
    }

    QVector2D toVector2D() const {
        return QVector2D(x, y);
    }

    // 静态帮助方法
    static float distance(const Vector2& a, const Vector2& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return qSqrt(dx * dx + dy * dy);
    }

    static float angle(const Vector2& a, const Vector2& b) {
        return qAcos((a * b) / (a.length() * b.length())) * 180.0f / M_PI;
    }
};

#endif // !_VECTOR2_QT_H_
