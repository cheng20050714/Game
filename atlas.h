#ifndef _ATLAS_H_
#define _ATLAS_H_

#include <QPixmap>
#include <QImage>
#include <QDir>
#include <vector>

class Atlas {
public:
    Atlas() = default;

    // 拷贝构造函数
    Atlas(const Atlas& other) {
        tex_list = other.tex_list;  // 复制所有 QPixmap
    }

    // 拷贝赋值运算符（可选）
    Atlas& operator=(const Atlas& other) {
        if (this != &other) {
            tex_list = other.tex_list;
        }
        return *this;
    }

    ~Atlas() = default; // Qt 的 QPixmap 会自动管理资源释放

    // 从格式化路径加载一系列图片
    void load(const QString& path_template, int num) {
        for (int i = 0; i < num; i++) {
            QString path_file = path_template.arg(i + 1);
            QPixmap pixmap(path_file);
            if (!pixmap.isNull()) {
                tex_list.push_back(pixmap);
            }
        }
    }

    // 加载单个文件夹中的所有图片（使用 QDir 扩展功能）
    void load_directory(const QString& directory, const QString& filter = "*.png") {
        QDir dir(directory);
        QStringList fileList = dir.entryList(QStringList() << filter, QDir::Files, QDir::Name);

        for (const QString& file : fileList) {
            QPixmap pixmap(dir.filePath(file));
            if (!pixmap.isNull()) {
                tex_list.push_back(pixmap);
            }
        }
    }

    // 清除所有图片
    void clear() {
        tex_list.clear();
    }

    // 获取图集大小
    int get_size() const {
        return static_cast<int>(tex_list.size());
    }

    // 获取指定索引的图片
    const QPixmap& get_texture(int idx) const {
        static QPixmap empty;
        if (idx < 0 || idx >= static_cast<int>(tex_list.size())) {
            return empty;
        }
        return tex_list[idx];
    }

    const std::vector<QPixmap>& get_textures() const {
        return tex_list;
    }

    // 添加纹理到图集
    void add_texture(const QPixmap& texture) {
        tex_list.push_back(texture);
    }

private:
    std::vector<QPixmap> tex_list;
};

#endif // !_ATLAS_QT_H_
