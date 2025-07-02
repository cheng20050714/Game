#include "mainwindow.h"
#include <QApplication>
#include <QPixmap>
#include <QSoundEffect>
#include "global_resources.h"  // 引入全局资源声明

// 在 main.cpp 中做唯一定义（不要在头文件定义）
QPixmap* tex_bullet = nullptr;
QSoundEffect* sound_explosion = nullptr;
Atlas* atlasChickenFast = nullptr;
Atlas* atlasChickenMedium = nullptr;
Atlas* atlasChickenSlow = nullptr;
Atlas* atlasExplosion = nullptr;

// 清理函数
void cleanupResources() {
    delete tex_bullet;
    delete sound_explosion;
    delete atlasChickenFast;
    delete atlasChickenMedium;
    delete atlasChickenSlow;
    delete atlasExplosion;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.loadResources();  // 在 MainWindow 中初始化全局资源
    w.show();

    int result = app.exec();
    cleanupResources();
    return result;
}
