# Qt6 QWidget 现代化 UI 设计完整指南
## 基于 Ant Design 5.x 设计体系 + QSS 实战技巧

> 项目主色: `#1677ff` | 成功: `#52c41a` | 警告: `#faad14` | 危险: `#ff4d4f`

---

## 一、Ant Design 5.x 设计令牌 (Design Tokens) 完整映射

### 1.1 色彩系统

#### 主色色阶 (Blue)
```
Blue-1:  #e6f4ff   ← 选中背景、hover 浅色
Blue-2:  #bae0ff   ← 浅色辅助
Blue-3:  #91caff   ← disabled 主按钮
Blue-4:  #69b1ff   ← 辅助
Blue-5:  #4096ff   ← hover 状态主色
Blue-6:  #1677ff   ← ★ 主色 (Primary)
Blue-7:  #0958d9   ← active/pressed 状态
Blue-8:  #003eb3   ← 深色辅助
Blue-9:  #002c8c   ← 深色辅助
Blue-10: #001d66   ← 最深
```

#### 功能色
```
Success:    #52c41a  (Green-6)
Success-H:  #73d13d  (Green-5, hover)
Success-D:  #389e0d  (Green-7, pressed)
Success-Bg: #f6ffed  (Green-1, 浅背景)

Warning:    #faad14  (Gold-6)
Warning-H:  #ffc53d  (Gold-5, hover)
Warning-D:  #d48806  (Gold-7, pressed)
Warning-Bg: #fffbe6  (Gold-1, 浅背景)

Error:      #ff4d4f  (Red-5)
Error-H:    #ff7875  (Red-4, hover)
Error-D:    #d9363e  (Red-7, pressed)
Error-Bg:   #fff2f0  (Red-1, 浅背景)

Info:       #1677ff  (Blue-6)
Info-Bg:    #e6f4ff  (Blue-1, 浅背景)
```

#### 中性色体系 (文本层级)
```
标题色:     #1f1f1f  (重要标题, 90%)
主文本:     #262626  (正文重要内容, 85%)
正文色:     #595959  (普通正文, 65%)
次要色:     #8c8c8c  (辅助说明, 45%)
禁用色:     #bfbfbf  (禁用文本, 25%)
边框色:     #d9d9d9  (输入框边框)
分割线:     #e8e8e8  (Hr/Border)
浅灰线:     #f0f0f0  (表格线、轻分割)
背景色:     #f5f5f5  (页面底色)
组件背景:   #fafafa  (表头、hover 行)
白色:       #ffffff  (卡片、容器)
```

### 1.2 间距系统 (4px 基准)

```
4xs:  2px    ← 图标与文字间距
3xs:  4px    ← 紧凑元素间距
2xs:  8px    ← 表单元素内间距、标签间距
xs:   12px   ← 列表项纵向间距
sm:   16px   ← 卡片内组件间距、模块间距
md:   20px   ← 页面边距、大模块间距
lg:   24px   ← 区块分隔
xl:   32px   ← 大型区域间距
2xl:  40px   ← 页面级大间距
3xl:  48px   ← 最大间距
```

### 1.3 字体系统

```
字体族: "Microsoft YaHei UI", "Segoe UI", "PingFang SC", sans-serif

辅助文字:   12px / line-height: 20px   ← 标签、时间戳
正文:       14px / line-height: 22px   ← 默认字号
小标题:     16px / line-height: 24px   ← 卡片标题
中标题:     20px / line-height: 28px   ← 页面标题
大标题:     24px / line-height: 32px   ← Dashboard 欢迎语
超大标题:   30px / line-height: 38px   ← 数据展示大数字
数据字体:   32px / line-height: 40px   ← 统计卡片数字

字重:
  常规: 400
  中等: 500  ← 按钮、选中tab
  半粗: 600  ← 表头、卡片标题
  粗体: 700  ← 重要标题
```

### 1.4 圆角系统

```
2px:  极小元素 (指示器、badge)
4px:  小元素 (tag、进度条、菜单)
6px:  交互控件 (按钮、输入框、下拉框)
8px:  容器 (卡片、表格、分组框、对话框)
10px: 大卡片 (dashboard stat card)
12px: 面板级容器
16px: 大型浮层
```

### 1.5 阴影层级 (Elevation)

> **重要**: QSS 不支持 `box-shadow`！必须用 `QGraphicsDropShadowEffect` 在 C++ 中实现。

```
Level 0: 无阴影 (贴地元素)
Level 1: 0 1px 2px 0 rgba(0,0,0,0.03), 0 1px 6px -1px rgba(0,0,0,0.02), 0 2px 4px 0 rgba(0,0,0,0.02)
         → 卡片、按钮默认
Level 2: 0 3px 6px -4px rgba(0,0,0,0.12), 0 6px 16px 0 rgba(0,0,0,0.08), 0 9px 28px 8px rgba(0,0,0,0.05)
         → 下拉菜单、弹出框
Level 3: 0 6px 16px -8px rgba(0,0,0,0.08), 0 9px 28px 0 rgba(0,0,0,0.05), 0 12px 48px 16px rgba(0,0,0,0.03)
         → 对话框、模态框
```

**C++ 实现阴影的通用方法：**

```cpp
#include <QGraphicsDropShadowEffect>

// 工具函数：为任意 QWidget 添加阴影
void applyShadow(QWidget* widget, int blurRadius = 20, int offsetY = 4,
                 const QColor& color = QColor(0, 0, 0, 15)) {
    auto* shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blurRadius);
    shadow->setColor(color);
    shadow->setOffset(0, offsetY);
    widget->setGraphicsEffect(shadow);
}

// 卡片默认阴影 (Level 1 - 轻柔)
applyShadow(card, 16, 2, QColor(0, 0, 0, 10));

// 弹出层阴影 (Level 2 - 中等)
applyShadow(popup, 24, 6, QColor(0, 0, 0, 30));

// 对话框阴影 (Level 3 - 强)
applyShadow(dialog, 32, 8, QColor(0, 0, 0, 40));
```

---

## 二、现代 QSS 技术实战

### 2.1 卡片容器 (Card Container)

#### 方法一：纯 QSS 模拟卡片 (简单场景)
```css
/* 基础卡片样式 - 通过边框 + 圆角模拟 */
QFrame[cssClass="card"] {
    background: #ffffff;
    border: 1px solid #f0f0f0;
    border-radius: 8px;
    padding: 0;
}
QFrame[cssClass="card"]:hover {
    border-color: #1677ff;
}
```

#### 方法二：C++ CardWidget 类 (推荐，支持真正阴影)
```cpp
// CardWidget.h
#pragma once
#include <QFrame>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

class CardWidget : public QFrame {
    Q_OBJECT
public:
    explicit CardWidget(QWidget* parent = nullptr)
        : QFrame(parent)
    {
        // 外层需要透明背景的父容器来容纳阴影
        setStyleSheet(R"(
            CardWidget {
                background: #ffffff;
                border: 1px solid #f0f0f0;
                border-radius: 8px;
            }
            CardWidget:hover {
                border-color: #d9d9d9;
            }
        )");

        // 阴影效果
        auto* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(16);
        shadow->setColor(QColor(0, 0, 0, 10));
        shadow->setOffset(0, 2);
        setGraphicsEffect(shadow);

        // 内容布局
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(20, 16, 20, 16);
        m_layout->setSpacing(8);
    }

    QVBoxLayout* contentLayout() { return m_layout; }

    void setTitle(const QString& title) {
        if (!m_titleLabel) {
            m_titleLabel = new QLabel(this);
            m_titleLabel->setStyleSheet(
                "font-size: 16px; font-weight: 600; color: #262626; "
                "background: transparent; border: none;");
            m_layout->insertWidget(0, m_titleLabel);
        }
        m_titleLabel->setText(title);
    }

private:
    QVBoxLayout* m_layout = nullptr;
    QLabel* m_titleLabel = nullptr;
};
```

**使用时外层需要透明容器：**
```cpp
// 创建带阴影的卡片 - 外层容器必须透明
auto* wrapper = new QWidget(parent);
wrapper->setStyleSheet("background: transparent;");
auto* wrapperLayout = new QVBoxLayout(wrapper);
wrapperLayout->setContentsMargins(8, 8, 8, 8); // margin 给阴影留空间

auto* card = new CardWidget(wrapper);
card->setTitle(QStringLiteral("统计概览"));
wrapperLayout->addWidget(card);
```

### 2.2 统计卡片 (Stat Card)

```cpp
QWidget* createStatCard(const QString& title, const QString& value,
                        const QString& subtitle, const QString& color,
                        QWidget* parent) {
    // 外层透明容器(阴影空间)
    auto* wrapper = new QWidget(parent);
    wrapper->setStyleSheet("background: transparent;");
    auto* wrapperLayout = new QVBoxLayout(wrapper);
    wrapperLayout->setContentsMargins(6, 6, 6, 10);

    auto* card = new QFrame(wrapper);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(QString(R"(
        QFrame {
            background: #ffffff;
            border: 1px solid #f0f0f0;
            border-radius: 10px;
        }
        QFrame:hover {
            border-color: %1;
        }
        QLabel {
            background: transparent;
            border: none;
        }
    )").arg(color));

    // 阴影
    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(16);
    shadow->setColor(QColor(0, 0, 0, 8));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(4);

    // 顶部色条
    auto* bar = new QFrame(card);
    bar->setFixedHeight(3);
    bar->setStyleSheet(QString(
        "background: %1; border-radius: 2px;").arg(color));
    layout->addWidget(bar);

    layout->addSpacing(4);

    // 标题
    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #8c8c8c; font-size: 13px;");
    layout->addWidget(titleLabel);

    // 数值
    auto* valueLabel = new QLabel(value);
    valueLabel->setStyleSheet(QString(
        "color: %1; font-size: 32px; font-weight: bold;").arg(color));
    layout->addWidget(valueLabel);

    // 副标题/趋势
    if (!subtitle.isEmpty()) {
        auto* subLabel = new QLabel(subtitle);
        subLabel->setStyleSheet("color: #8c8c8c; font-size: 12px;");
        layout->addWidget(subLabel);
    }

    layout->addStretch();
    return wrapper;
}
```

### 2.3 现代表格样式 (QTableWidget)

#### 完整表格 QSS
```css
/* === 表格容器 === */
QTableWidget {
    background: #ffffff;
    border: none;
    border-radius: 8px;
    gridline-color: transparent;        /* 隐藏网格线，用 border-bottom 替代 */
    selection-background-color: #e6f4ff;
    selection-color: #1f1f1f;
    alternate-background-color: #fafafa; /* 交替行颜色 */
    outline: none;                       /* 去掉焦点虚线框 */
    font-size: 13px;
}

/* === 单元格 === */
QTableWidget::item {
    padding: 10px 12px;
    border-bottom: 1px solid #f5f5f5;
}
QTableWidget::item:hover {
    background: #e6f4ff;
}
QTableWidget::item:selected {
    background: #e6f4ff;
    color: #1677ff;
}

/* === 表头 === */
QHeaderView::section {
    background: #fafafa;
    color: #8c8c8c;
    padding: 10px 12px;
    border: none;
    border-bottom: 2px solid #f0f0f0;
    border-right: 1px solid #f5f5f5;
    font-weight: 600;
    font-size: 12px;
}
QHeaderView::section:last {
    border-right: none;
}
QHeaderView::section:hover {
    background: #f0f0f0;
    color: #595959;
}

/* 排序箭头 */
QHeaderView::down-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid #8c8c8c;
    margin-left: 4px;
}
QHeaderView::up-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-bottom: 5px solid #8c8c8c;
    margin-left: 4px;
}
```

#### 表格搜索/筛选栏模式
```cpp
// 创建表格上方的工具栏
QWidget* createTableToolbar(QWidget* parent, QLineEdit*& searchInput,
                            QPushButton*& refreshBtn, QPushButton*& addBtn) {
    auto* toolbar = new QWidget(parent);
    toolbar->setStyleSheet("background: transparent;");
    auto* layout = new QHBoxLayout(toolbar);
    layout->setContentsMargins(0, 0, 0, 12);
    layout->setSpacing(12);

    // 搜索框
    searchInput = new QLineEdit(toolbar);
    searchInput->setPlaceholderText(QStringLiteral("搜索..."));
    searchInput->setFixedWidth(260);
    searchInput->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #d9d9d9;
            border-radius: 6px;
            padding: 6px 12px 6px 32px;
            background: #ffffff;
            font-size: 14px;
        }
        QLineEdit:hover { border-color: #4096ff; }
        QLineEdit:focus { border-color: #1677ff; }
    )");
    layout->addWidget(searchInput);

    layout->addStretch();

    // 刷新按钮
    refreshBtn = new QPushButton(QStringLiteral("刷新"), toolbar);
    layout->addWidget(refreshBtn);

    // 新增按钮 (主按钮样式)
    addBtn = new QPushButton(QStringLiteral("+ 新增"), toolbar);
    addBtn->setProperty("cssClass", "primary");
    layout->addWidget(addBtn);

    return toolbar;
}
```

#### 表格排序实现
```cpp
// 启用表格排序
table->setSortingEnabled(true);
table->horizontalHeader()->setSortIndicatorShown(true);
table->horizontalHeader()->setSectionsClickable(true);

// 自定义排序（数字列按数值排序而非字符串）
// 需要继承 QTableWidgetItem
class NumericTableWidgetItem : public QTableWidgetItem {
public:
    NumericTableWidgetItem(const QString& text, int value)
        : QTableWidgetItem(text), m_value(value) {}

    bool operator<(const QTableWidgetItem& other) const override {
        auto* otherNum = dynamic_cast<const NumericTableWidgetItem*>(&other);
        if (otherNum) return m_value < otherNum->m_value;
        return QTableWidgetItem::operator<(other);
    }
private:
    int m_value;
};
```

### 2.4 现代标签页 (QTabWidget)

#### Ant Design 风格卡片式 Tab
```css
/* 标签页容器 */
QTabWidget::pane {
    border: 1px solid #e8e8e8;
    border-radius: 0 0 8px 8px;
    background: #ffffff;
    border-top: none;
    top: -1px;
}

/* 标签栏 */
QTabBar {
    background: transparent;
}

/* 单个标签 */
QTabBar::tab {
    background: transparent;
    color: #595959;
    padding: 12px 20px;
    border: none;
    border-bottom: 2px solid transparent;
    margin-right: 8px;
    font-size: 14px;
    min-width: 80px;
}

QTabBar::tab:hover {
    color: #1677ff;
}

QTabBar::tab:selected {
    color: #1677ff;
    border-bottom: 2px solid #1677ff;
    font-weight: 500;
}

/* 卡片式 Tab (另一种风格) */
QTabWidget[cssClass="card-tab"]::pane {
    border: 1px solid #e8e8e8;
    border-radius: 0 8px 8px 8px;
    background: #ffffff;
}
QTabWidget[cssClass="card-tab"] QTabBar::tab {
    background: #fafafa;
    border: 1px solid #e8e8e8;
    border-bottom: none;
    border-radius: 8px 8px 0 0;
    margin-right: 2px;
    padding: 10px 20px;
}
QTabWidget[cssClass="card-tab"] QTabBar::tab:selected {
    background: #ffffff;
    border-bottom: 1px solid #ffffff;
    margin-bottom: -1px;
}
```

### 2.5 按钮系统 (完整)

```css
/* === 默认按钮 === */
QPushButton {
    border-radius: 6px;
    padding: 7px 16px;
    font-size: 14px;
    font-weight: 500;
    background: #ffffff;
    color: #1f1f1f;
    border: 1px solid #d9d9d9;
    min-height: 20px;
}
QPushButton:hover {
    color: #4096ff;
    border-color: #4096ff;
}
QPushButton:pressed {
    color: #0958d9;
    border-color: #0958d9;
}
QPushButton:disabled {
    background: #f5f5f5;
    color: #bfbfbf;
    border-color: #d9d9d9;
}

/* === 主按钮 === */
QPushButton[cssClass="primary"] {
    background: #1677ff;
    color: #ffffff;
    border: 1px solid #1677ff;
}
QPushButton[cssClass="primary"]:hover {
    background: #4096ff;
    border-color: #4096ff;
}
QPushButton[cssClass="primary"]:pressed {
    background: #0958d9;
    border-color: #0958d9;
}
QPushButton[cssClass="primary"]:disabled {
    background: #91caff;
    border-color: #91caff;
    color: rgba(255,255,255,0.7);
}

/* === 成功按钮 === */
QPushButton[cssClass="success"] {
    background: #52c41a;
    color: #ffffff;
    border: 1px solid #52c41a;
}
QPushButton[cssClass="success"]:hover {
    background: #73d13d;
    border-color: #73d13d;
}

/* === 危险按钮 === */
QPushButton[cssClass="danger"] {
    background: #ff4d4f;
    color: #ffffff;
    border: 1px solid #ff4d4f;
}
QPushButton[cssClass="danger"]:hover {
    background: #ff7875;
    border-color: #ff7875;
}

/* === 虚线按钮 === */
QPushButton[cssClass="dashed"] {
    border: 1px dashed #d9d9d9;
    background: transparent;
    color: #595959;
}
QPushButton[cssClass="dashed"]:hover {
    border-color: #1677ff;
    color: #1677ff;
}

/* === 文字按钮 (无边框) === */
QPushButton[cssClass="text"] {
    background: transparent;
    border: none;
    color: #1677ff;
    padding: 4px 8px;
}
QPushButton[cssClass="text"]:hover {
    background: #e6f4ff;
}

/* === 链接按钮 === */
QPushButton[cssClass="link"] {
    background: transparent;
    border: none;
    color: #1677ff;
    padding: 4px 0;
}
QPushButton[cssClass="link"]:hover {
    color: #4096ff;
}

/* === 按钮尺寸 === */
QPushButton[cssClass="small"] {
    padding: 3px 8px;
    font-size: 12px;
    min-height: 16px;
}
QPushButton[cssClass="large"] {
    padding: 10px 24px;
    font-size: 16px;
    min-height: 24px;
}

/* === 图标按钮 (圆形) === */
QPushButton[cssClass="icon"] {
    border-radius: 16px;
    padding: 6px;
    min-width: 20px;
    max-width: 20px;
    min-height: 20px;
    max-height: 20px;
    border: 1px solid #d9d9d9;
    background: #ffffff;
}
```

**C++ 中使用 property 设置：**
```cpp
auto* btn = new QPushButton(QStringLiteral("保存"));
btn->setProperty("cssClass", "primary");

auto* dangerBtn = new QPushButton(QStringLiteral("删除"));
dangerBtn->setProperty("cssClass", "danger");

// 小按钮
auto* smallBtn = new QPushButton(QStringLiteral("编辑"));
smallBtn->setProperty("cssClass", "text");
smallBtn->setProperty("cssClass", "small"); // 组合需要代码中合并
```

### 2.6 输入框系统 (完整)

```css
/* === 单行输入框 === */
QLineEdit {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 6px 12px;
    background: #ffffff;
    color: #1f1f1f;
    selection-background-color: #1677ff;
    selection-color: #ffffff;
    font-size: 14px;
}
QLineEdit:hover {
    border-color: #4096ff;
}
QLineEdit:focus {
    border-color: #1677ff;
    /* QSS 不支持 box-shadow, 需要用 C++ 实现 */
}
QLineEdit:disabled {
    background: #f5f5f5;
    color: #bfbfbf;
}
QLineEdit[readOnly="true"] {
    background: #f5f5f5;
    color: #595959;
}
/* 错误状态 (C++ 中设置 property) */
QLineEdit[cssClass="error"] {
    border-color: #ff4d4f;
}
QLineEdit[cssClass="error"]:focus {
    border-color: #ff4d4f;
}

/* === 多行输入框 === */
QTextEdit {
    border: 1px solid #d9d9d9;
    border-radius: 6px;
    padding: 8px 12px;
    background: #ffffff;
    color: #1f1f1f;
    selection-background-color: #1677ff;
    selection-color: #ffffff;
}
QTextEdit:hover { border-color: #4096ff; }
QTextEdit:focus { border-color: #1677ff; }

/* === 大输入框 === */
QLineEdit[cssClass="large"] {
    padding: 10px 16px;
    font-size: 16px;
    border-radius: 8px;
}

/* === 搜索输入框 === */
QLineEdit[cssClass="search"] {
    padding: 8px 12px 8px 36px;  /* 左侧留空给搜索图标 */
    border-radius: 20px;          /* 圆角搜索框 */
}
```

**输入框 focus 光晕效果 (C++)：**
```cpp
// 为 QLineEdit 添加 focus 时的光晕效果
void addFocusGlow(QLineEdit* input) {
    auto* glow = new QGraphicsDropShadowEffect(input);
    glow->setBlurRadius(8);
    glow->setColor(QColor(22, 119, 255, 38));  // rgba(22,119,255,0.15)
    glow->setOffset(0, 0);

    // 默认隐藏光晕
    glow->setEnabled(false);
    input->setGraphicsEffect(glow);

    // 监听焦点事件
    input->installEventFilter(new QObject(input));
    QObject::connect(input, &QLineEdit::textChanged, [input, glow]() {});

    // 使用 eventFilter 来切换
    class FocusFilter : public QObject {
    public:
        FocusFilter(QGraphicsDropShadowEffect* g) : glow(g) {}
    protected:
        bool eventFilter(QObject* obj, QEvent* event) override {
            if (event->type() == QEvent::FocusIn) {
                glow->setEnabled(true);
            } else if (event->type() == QEvent::FocusOut) {
                glow->setEnabled(false);
            }
            return false;
        }
    private:
        QGraphicsDropShadowEffect* glow;
    };
    input->installEventFilter(new FocusFilter(glow));
}
```

### 2.7 状态标签/徽章 (Tag / Badge)

#### QSS 中用 QLabel + property 模拟 Tag
```cpp
// 创建状态标签的工厂函数
QLabel* createTag(const QString& text, const QString& type, QWidget* parent) {
    auto* tag = new QLabel(text, parent);
    tag->setAlignment(Qt::AlignCenter);
    tag->setFixedHeight(22);

    // 根据类型设置颜色
    QString bg, color, border;
    if (type == "success") {
        bg = "#f6ffed"; color = "#52c41a"; border = "#b7eb8f";
    } else if (type == "warning") {
        bg = "#fffbe6"; color = "#faad14"; border = "#ffe58f";
    } else if (type == "error") {
        bg = "#fff2f0"; color = "#ff4d4f"; border = "#ffccc7";
    } else if (type == "info") {
        bg = "#e6f4ff"; color = "#1677ff"; border = "#91caff";
    } else if (type == "default") {
        bg = "#fafafa"; color = "#595959"; border = "#d9d9d9";
    } else if (type == "processing") {
        bg = "#e6f4ff"; color = "#1677ff"; border = "#91caff";
    }

    tag->setStyleSheet(QString(
        "QLabel {"
        "  background: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 2px 8px;"
        "  font-size: 12px;"
        "}"
    ).arg(bg, color, border));

    return tag;
}
```

#### 在表格中使用状态标签
```cpp
// 用 QTableWidgetItem 的自定义 widget 或者 HTML 渲染
// 方法1：HTML 渲染（简单）
auto* item = new QTableWidgetItem();
item->setText(QString(
    "<html><body>"
    "<span style='background:#f6ffed; color:#52c41a; "
    "border:1px solid #b7eb8f; border-radius:4px; "
    "padding:2px 8px; font-size:12px;'>正常</span>"
    "</body></html>"
));

// 方法2：setCellWidget (更灵活)
auto* tag = createTag(QStringLiteral("处理中"), "processing", table);
table->setCellWidget(row, col, tag);
```

#### 数字徽章 (Badge)
```cpp
QLabel* createBadge(int count, QWidget* parent) {
    auto* badge = new QLabel(count > 99 ? "99+" : QString::number(count), parent);
    badge->setAlignment(Qt::AlignCenter);
    badge->setMinimumSize(18, 18);
    badge->setStyleSheet(R"(
        QLabel {
            background: #ff4d4f;
            color: #ffffff;
            border-radius: 9px;
            font-size: 11px;
            font-weight: 500;
            padding: 0 5px;
            border: 2px solid #ffffff;
        }
    )");
    return badge;
}
```

### 2.8 滚动区域 (QScrollArea)

```css
QScrollArea {
    border: none;
    background: transparent;
}

/* 垂直滚动条 */
QScrollBar:vertical {
    width: 6px;
    background: transparent;
    margin: 0;
    border: none;
}
QScrollBar::handle:vertical {
    background: rgba(0, 0, 0, 0.12);
    border-radius: 3px;
    min-height: 40px;
}
QScrollBar::handle:vertical:hover {
    background: rgba(0, 0, 0, 0.25);
}
QScrollBar::handle:vertical:pressed {
    background: rgba(0, 0, 0, 0.4);
}
QScrollBar::add-line:vertical,
QScrollBar::sub-line:vertical {
    height: 0;
    border: none;
}
QScrollBar::add-page:vertical,
QScrollBar::sub-page:vertical {
    background: transparent;
}

/* 水平滚动条 */
QScrollBar:horizontal {
    height: 6px;
    background: transparent;
    margin: 0;
    border: none;
}
QScrollBar::handle:horizontal {
    background: rgba(0, 0, 0, 0.12);
    border-radius: 3px;
    min-width: 40px;
}
QScrollBar::handle:horizontal:hover {
    background: rgba(0, 0, 0, 0.25);
}
QScrollBar::add-line:horizontal,
QScrollBar::sub-line:horizontal {
    width: 0;
    border: none;
}
QScrollBar::add-page:horizontal,
QScrollBar::sub-page:horizontal {
    background: transparent;
}
```

---

## 三、Qt 特有 UI 增强模式

### 3.1 响应式布局

```cpp
// 使用 QSplitter 实现可调整大小的面板
#include <QSplitter>

auto* splitter = new QSplitter(Qt::Horizontal, parent);
splitter->setHandleWidth(1);
splitter->setStyleSheet("QSplitter::handle { background: #e8e8e8; }");
splitter->addWidget(leftWidget);
splitter->addWidget(rightWidget);
splitter->setStretchFactor(0, 3);  // 左侧 60%
splitter->setStretchFactor(1, 2);  // 右侧 40%

// 自适应网格 - 根据窗口宽度自动调整列数
void setupResponsiveGrid(QGridLayout* grid, int itemCount) {
    // 在 resizeEvent 中动态调整
    int width = parentWidget->width();
    int cols = width > 1400 ? 4 : width > 1000 ? 3 : width > 700 ? 2 : 1;
    // 重新排列子控件
}
```

### 3.2 空状态设计 (Empty State)

```cpp
QWidget* createEmptyState(const QString& icon, const QString& title,
                          const QString& description, QWidget* parent) {
    auto* widget = new QWidget(parent);
    widget->setStyleSheet("background: transparent;");
    auto* layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(12);

    // 图标 (大号)
    auto* iconLabel = new QLabel(icon, widget);
    iconLabel->setStyleSheet("font-size: 48px; color: #d9d9d9;");
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    // 标题
    auto* titleLabel = new QLabel(title, widget);
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: 500; color: #595959;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // 描述
    if (!description.isEmpty()) {
        auto* descLabel = new QLabel(description, widget);
        descLabel->setStyleSheet(
            "font-size: 14px; color: #8c8c8c;");
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setWordWrap(true);
        layout->addWidget(descLabel);
    }

    layout->addSpacing(8);
    return widget;
}

// 使用示例
auto* empty = createEmptyState(
    QStringLiteral("📋"),
    QStringLiteral("暂无数据"),
    QStringLiteral("当前没有可显示的内容"),
    parent);
```

### 3.3 加载状态指示器

```cpp
// 方法1：简单的加载文字
auto* loadingLabel = new QLabel(QStringLiteral("加载中..."), parent);
loadingLabel->setStyleSheet(
    "color: #8c8c8c; font-size: 14px; padding: 40px;");
loadingLabel->setAlignment(Qt::AlignCenter);

// 方法2：使用 QMovie 播放 GIF 加载动画
#include <QMovie>
auto* movie = new QMovie(":/loading.gif");
auto* loadingIndicator = new QLabel(parent);
loadingIndicator->setMovie(movie);
loadingIndicator->setAlignment(Qt::AlignCenter);
movie->start();

// 方法3：使用 QProgressBar 不确定进度
auto* progressBar = new QProgressBar(parent);
progressBar->setRange(0, 0);  // 0,0 表示不确定模式
progressBar->setFixedHeight(4);
progressBar->setStyleSheet(R"(
    QProgressBar {
        border: none;
        background: #f0f0f0;
        border-radius: 2px;
    }
    QProgressBar::chunk {
        background: #1677ff;
        border-radius: 2px;
    }
)");
```

### 3.4 侧边栏导航样式 (完整版)

```css
/* 侧边栏容器 */
QWidget#sidebar {
    background: #001529;
}

/* 导航项 */
QPushButton[cssClass="nav-item"] {
    background: transparent;
    color: rgba(255, 255, 255, 0.70);
    border: none;
    border-left: 3px solid transparent;
    padding: 12px 20px;
    text-align: left;
    font-size: 14px;
    min-height: 40px;
    border-radius: 0;
}
QPushButton[cssClass="nav-item"]:hover {
    background: rgba(255, 255, 255, 0.08);
    color: #ffffff;
    border-left: 3px solid #1677ff;
}
QPushButton[cssClass="nav-item"][active="true"] {
    background: rgba(22, 119, 255, 0.15);
    color: #ffffff;
    font-weight: 600;
    border-left: 3px solid #1677ff;
}

/* 分组标题 */
QPushButton[cssClass="nav-group"] {
    background: transparent;
    color: rgba(255, 255, 255, 0.45);
    border: none;
    padding: 12px 20px;
    text-align: left;
    font-size: 12px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 1px;
}
```

---

## 四、完整页面布局模板

### 4.1 列表页模板 (搜索 + 表格 + 分页)

```cpp
QWidget* createListPage(const QString& title, QWidget* parent) {
    auto* page = new QWidget(parent);
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 24, 24, 24);
    pageLayout->setSpacing(16);

    // === 页面标题 ===
    auto* titleLabel = new QLabel(title, page);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: 600; color: #1f1f1f; "
        "background: transparent;");
    pageLayout->addWidget(titleLabel);

    // === 搜索卡片 ===
    auto* searchCard = new QFrame(page);
    searchCard->setStyleSheet(
        "QFrame { background: #fff; border: 1px solid #f0f0f0; "
        "border-radius: 8px; } "
        "QLabel { background: transparent; border: none; }");
    auto* searchLayout = new QHBoxLayout(searchCard);
    searchLayout->setContentsMargins(20, 16, 20, 16);
    searchLayout->setSpacing(12);

    // 搜索字段
    auto* nameLabel = new QLabel(QStringLiteral("名称:"));
    nameLabel->setStyleSheet("color: #595959; font-size: 14px;");
    searchLayout->addWidget(nameLabel);

    auto* nameInput = new QLineEdit();
    nameInput->setPlaceholderText(QStringLiteral("请输入名称"));
    nameInput->setFixedWidth(200);
    searchLayout->addWidget(nameInput);

    // 状态下拉
    auto* statusLabel = new QLabel(QStringLiteral("状态:"));
    statusLabel->setStyleSheet("color: #595959; font-size: 14px;");
    searchLayout->addWidget(statusLabel);

    auto* statusCombo = new QComboBox();
    statusCombo->addItem(QStringLiteral("全部"));
    statusCombo->addItem(QStringLiteral("正常"));
    statusCombo->addItem(QStringLiteral("禁用"));
    statusCombo->setFixedWidth(140);
    searchLayout->addWidget(statusCombo);

    searchLayout->addStretch();

    // 查询按钮
    auto* searchBtn = new QPushButton(QStringLiteral("查询"));
    searchBtn->setProperty("cssClass", "primary");
    searchLayout->addWidget(searchBtn);

    // 重置按钮
    auto* resetBtn = new QPushButton(QStringLiteral("重置"));
    searchLayout->addWidget(resetBtn);

    pageLayout->addWidget(searchCard);

    // === 表格卡片 ===
    auto* tableCard = new QFrame(page);
    tableCard->setStyleSheet(
        "QFrame { background: #fff; border: 1px solid #f0f0f0; "
        "border-radius: 8px; }");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(0, 16, 0, 0);

    // 表格工具栏
    auto* tableToolbar = new QWidget(tableCard);
    tableToolbar->setStyleSheet("background: transparent;");
    auto* toolbarLayout = new QHBoxLayout(tableToolbar);
    toolbarLayout->setContentsMargins(20, 0, 20, 12);

    auto* addBtn = new QPushButton(QStringLiteral("+ 新增"));
    addBtn->setProperty("cssClass", "primary");
    toolbarLayout->addWidget(addBtn);

    auto* batchDeleteBtn = new QPushButton(QStringLiteral("批量删除"));
    batchDeleteBtn->setProperty("cssClass", "danger");
    toolbarLayout->addWidget(batchDeleteBtn);

    toolbarLayout->addStretch();
    tableLayout->addWidget(tableToolbar);

    // 表格
    auto* table = new QTableWidget(tableCard);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setSortingEnabled(true);
    // 应用表格样式...
    tableLayout->addWidget(table);

    pageLayout->addWidget(tableCard);

    return page;
}
```

### 4.2 详情页/表单页模板

```cpp
QWidget* createFormPage(const QString& title, QWidget* parent) {
    auto* scrollArea = new QScrollArea(parent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: #f5f5f5; }");

    auto* page = new QWidget();
    auto* pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(24, 24, 24, 24);
    pageLayout->setSpacing(16);

    // 页面标题
    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: 600; color: #1f1f1f; "
        "background: transparent;");
    pageLayout->addWidget(titleLabel);

    // 表单卡片
    auto* formCard = new QFrame(page);
    formCard->setStyleSheet(
        "QFrame { background: #fff; border: 1px solid #f0f0f0; "
        "border-radius: 8px; } "
        "QLabel { background: transparent; border: none; }");
    auto* formOuterLayout = new QVBoxLayout(formCard);
    formOuterLayout->setContentsMargins(24, 20, 24, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setHorizontalSpacing(16);
    formLayout->setVerticalSpacing(20);

    // 表单项
    auto* nameInput = new QLineEdit();
    nameInput->setFixedWidth(320);
    formLayout->addRow(QStringLiteral("名称:"), nameInput);

    auto* descInput = new QTextEdit();
    descInput->setFixedWidth(320);
    descInput->setFixedHeight(100);
    formLayout->addRow(QStringLiteral("描述:"), descInput);

    formOuterLayout->addLayout(formLayout);

    // 底部操作栏
    formOuterLayout->addSpacing(16);
    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #e8e8e8; max-height: 1px;");
    formOuterLayout->addWidget(separator);

    formOuterLayout->addSpacing(8);
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton(QStringLiteral("取消"));
    btnLayout->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(QStringLiteral("保存"));
    saveBtn->setProperty("cssClass", "primary");
    btnLayout->addWidget(saveBtn);

    formOuterLayout->addLayout(btnLayout);
    pageLayout->addWidget(formCard);
    pageLayout->addStretch();

    scrollArea->setWidget(page);
    return scrollArea;
}
```

---

## 五、组件密度标准 (Ant Design 5.x)

### 控件高度规范
```
小尺寸 (small):
  按钮: 24px 高度 (padding: 3px 8px)
  输入框: 24px 高度 (padding: 3px 8px)

默认尺寸 (default):
  按钮: 32px 高度 (padding: 7px 16px)
  输入框: 32px 高度 (padding: 6px 12px)
  下拉框: 32px 高度
  表格行: 40px 高度 (padding: 10px 12px)

大尺寸 (large):
  按钮: 40px 高度 (padding: 10px 24px)
  输入框: 40px 高度 (padding: 10px 16px)
```

### 容器内边距
```
卡片: 20px/16px (水平/垂直)
页面: 24px
表格外边距: 0 (在卡片内撑满)
分组间距: 16px
```

---

## 六、C++ 工具函数库

### 6.1 统一的卡片创建工具

```cpp
// 创建标准白卡片
QFrame* createCard(QWidget* parent, bool withShadow = true) {
    auto* wrapper = new QWidget(parent);
    wrapper->setStyleSheet("background: transparent;");
    auto* wrapperLayout = new QVBoxLayout(wrapper);
    int margin = withShadow ? 8 : 0;
    wrapperLayout->setContentsMargins(margin, margin, margin, margin + 4);

    auto* card = new QFrame(wrapper);
    card->setStyleSheet(R"(
        QFrame {
            background: #ffffff;
            border: 1px solid #f0f0f0;
            border-radius: 8px;
        }
        QLabel {
            background: transparent;
            border: none;
        }
    )");

    if (withShadow) {
        auto* shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(16);
        shadow->setColor(QColor(0, 0, 0, 8));
        shadow->setOffset(0, 2);
        card->setGraphicsEffect(shadow);
    }

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(8);
    card->setProperty("_layout", QVariant::fromValue(layout));

    wrapperLayout->addWidget(card);
    return card;
}
```

### 6.2 分割线

```cpp
QFrame* createDivider(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e8e8e8; max-height: 1px; background: #e8e8e8;");
    return line;
}

QFrame* createVerticalDivider(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFrameShape(QFrame::VLine);
    line->setFixedWidth(1);
    line->setStyleSheet("color: #e8e8e8; background: #e8e8e8;");
    return line;
}
```

### 6.3 页面头部面包屑

```cpp
QWidget* createBreadcrumb(const QStringList& items, QWidget* parent) {
    auto* widget = new QWidget(parent);
    widget->setStyleSheet("background: transparent;");
    auto* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    for (int i = 0; i < items.size(); ++i) {
        auto* label = new QLabel(items[i]);
        bool isLast = (i == items.size() - 1);
        label->setStyleSheet(isLast
            ? "color: #1f1f1f; font-size: 13px;"
            : "color: #8c8c8c; font-size: 13px;");
        layout->addWidget(label);

        if (!isLast) {
            auto* sep = new QLabel(QStringLiteral(" / "));
            sep->setStyleSheet("color: #d9d9d9; font-size: 13px;");
            layout->addWidget(sep);
        }
    }
    layout->addStretch();
    return widget;
}
```

---

## 七、重要注意事项

### QSS 限制与解决方案

| 特性 | QSS 支持 | 替代方案 |
|------|---------|---------|
| box-shadow | ❌ 不支持 | `QGraphicsDropShadowEffect` |
| transition/animation | ❌ 不支持 | `QPropertyAnimation` |
| transform | ❌ 不支持 | 自定义 `paintEvent` |
| rgba() in background | ✅ 支持 | — |
| gradient | ✅ 支持 | `qlineargradient()` |
| border-image | ✅ 支持 | — |
| ::before / ::after | ❌ 不支持 | 自定义 paint |
| text-overflow: ellipsis | ❌ 不支持 | `elideMode` 属性 |

### 使用 Fusion 基础样式
```cpp
// 在 main.cpp 中设置 Fusion 样式作为基础
QApplication::setStyle("Fusion");
// 然后加载自定义 QSS
QFile qss(":/style.qss");
if (qss.open(QFile::ReadOnly)) {
    app.setStyleSheet(qss.readAll());
}
```

### 动态属性刷新
```cpp
// 修改 property 后必须刷新样式
widget->setProperty("cssClass", "primary");
widget->style()->unpolish(widget);  // 必须先 unpolish
widget->style()->polish(widget);    // 再 polish
widget->update();
```

### 避免全局样式冲突
```cpp
// 使用 objectName 限定作用域
m_topBar->setObjectName("topBar");
// QSS 中:
// QWidget#topBar QPushButton { ... }

// 使用 property selector 分类
// QPushButton[cssClass="primary"] { ... }

// 避免使用纯 QWidget {} 选择器，改用:
// QWidget#centralWidget { ... }
```
