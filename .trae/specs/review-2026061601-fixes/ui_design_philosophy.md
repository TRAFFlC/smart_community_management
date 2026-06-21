# 智慧社区管理平台 — UI 设计理念文档

> 面向开发者的视觉设计规范，可直接指导 Qt6/QSS 实现。
> 版本：1.0 | 更新：2026-06-16

---

## 1. 设计定位

**风格**：现代轻量化政务/办公风——介于「管理后台」与「效率工具」之间，既要有政务系统的可信赖感，又要有飞书/钉钉桌面端的轻快体验。

**关键词**：清晰 · 克制 · 高效 · 柔和 · 可信赖

**参考基准**：

- **飞书桌面端** — 信息密度与留白的平衡、卡片式布局
- **Notion** — 内容优先、弱化装饰、专注数据呈现
- **Ant Design Pro** — 完整的企业级设计语言、配色与组件体系

**核心判断**：这是一个面向社区工作人员的日常办公工具，用户每天长时间使用。因此：

- 不追求视觉冲击，追求**长时间使用不疲劳**
- 不堆砌装饰，追求**信息获取效率**
- 不花哨动效，追求**操作确定性**

---

## 2. 设计原则

基于人机交互经典法则，结合本项目场景落地：

### Aesthetic-Usability Effect — 精致即易用

用户会本能地认为精致的界面更好用。这不是追求花哨，而是消除粗糙感：对齐的网格、一致的间距、克制的配色，让界面「看起来专业」从而「用起来放心」。

**落地**：统一卡片圆角 8px、统一边框色 `#f0f0f0`、消除文本框多余边框。

### Jakob's Law — 遵循用户已知的模式

用户大部分时间在使用其他应用，他们期望你的应用和他们已知的模式一致。采用 Ant Design 风格不是因为它是最好的，而是因为它是国内办公用户最熟悉的。

**落地**：侧边栏深色 `#001529`、主色 `#1677ff`、表格斑马纹——这些都是 Ant Design 用户的心智模型。

### Progressive Disclosure — 渐进式披露

不要一次把所有信息塞给用户。先展示概要，细节按需展开。

**落地**：统计卡片只显示核心数字，点击可跳转详情页；侧边栏手风琴模式同时只展开一个分组；表格默认显示核心列，更多列通过横向滚动查看。

### Proximity + Similarity — 相关元素视觉分组

物理距离近的元素被视为一组，外观相似的元素被视为同类。利用这两个原则组织信息层级。

**落地**：统计卡片之间间距 16px，卡片内部边距 20px——外松内紧；同类状态标签使用相同形状（圆角小色块），仅通过颜色区分含义。

### Miller's Law — 7±2 分块

人的工作记忆约 7 项。超过时需要分块。

**落地**：统计卡片一行不超过 4-5 个；侧边栏分组不超过 6 个子项；表格列数控制在 6-8 列以内，超出用详情弹窗。

### Doherty Threshold — 400ms 内反馈

操作后 400ms 内必须有视觉反馈，否则用户会认为系统没响应。

**落地**：按钮 hover/click 即时变色；数据加载时显示等待动画；删除操作前弹出确认框（也是反馈）。

### Wayfinding — 用户始终知道自己在哪

用户需要时刻知道：我在哪个页面、我能去哪里、我怎么回去。

**落地**：面包屑导航随页面切换实时更新；侧边栏当前项高亮蓝色；页面标题与面包屑一致。

---

## 3. 配色体系

基于 Ant Design 5.x 色板，针对桌面端场景微调。

### 功能色

| 用途     | 色值      | 说明                           |
| -------- | --------- | ------------------------------ |
| 品牌主色 | `#1677FF` | 主按钮、选中态、链接、活跃指示 |
| 主色悬浮 | `#4096FF` | hover 态                       |
| 主色按下 | `#0958D9` | pressed 态                     |
| 主色浅底 | `#E6F4FF` | 选中行背景、信息提示底色       |
| 成功色   | `#52C41A` | 通过、完成、已办结             |
| 警告色   | `#FA8C16` | 提醒、待处理、进行中           |
| 危险色   | `#FF4D4F` | 删除、逾期、异常               |
| 紫色     | `#722ED1` | 辅助色（服务类指标）           |
| 青色     | `#13C2C2` | 辅助色（公告类指标）           |

### 中性色

| 用途      | 色值      | 说明                 |
| --------- | --------- | -------------------- |
| 全局背景  | `#F0F2F5` | 页面底色，提供层次感 |
| 卡片背景  | `#FFFFFF` | 纯白，内容容器       |
| 正文文字  | `#262626` | 深灰黑，主文本       |
| 次级文字  | `#8C8C8C` | 中灰，辅助说明、标签 |
| 占位文字  | `#BFBFBF` | 浅灰，placeholder    |
| 边框/分割 | `#E8E8E8` | 浅灰线，分隔区域     |
| 卡片边框  | `#F0F0F0` | 极浅灰，卡片轮廓     |
| 表头背景  | `#FAFAFA` | 微灰，与内容区区分   |
| 斑马纹行  | `#FAFAFA` | 交替行背景           |
| 选中行    | `#E6F4FF` | 主色浅底             |

### 侧边栏专用色

| 用途         | 色值                     |
| ------------ | ------------------------ |
| 侧边栏背景   | `#001529`                |
| Logo 区域    | `#002140`                |
| 菜单文字     | `rgba(255,255,255,0.70)` |
| 菜单悬浮文字 | `#FFFFFF`                |
| 菜单悬浮背景 | `rgba(255,255,255,0.06)` |
| 活跃菜单背景 | `#1677FF`                |

### 配色原则

1. **一个页面不超过 3 种功能色**——主色 + 1 种状态色 + 中性色
2. **大面积用中性色，功能色只做点缀**——背景、文字、边框都是中性色，只有按钮、标签、指标数字用功能色
3. **同义状态同色**——所有「待处理」统一橙色，所有「已完成」统一绿色，不因页面不同而换色

---

## 4. 字体体系

### 字体栈

```css
/* QSS 全局设置 */
font-family: 'Microsoft YaHei UI', 'Microsoft YaHei', 'Segoe UI', sans-serif;
```

- 中文：Microsoft YaHei UI (Windows) / PingFang SC (macOS)
- 英文数字：Segoe UI (Windows) / SF Pro (macOS)
- `Microsoft YaHei UI` 优先于 `Microsoft YaHei`，前者在 Windows 上渲染更清晰

### 字号层级

| 层级     | 字号    | 字重          | 用途                             |
| -------- | ------- | ------------- | -------------------------------- |
| 页面标题 | 18-20px | Bold (700)    | 页面顶部标题，如「工单统计分析」 |
| 模块标题 | 16px    | Bold (700)    | 卡片标题、分组标题               |
| 正文     | 14px    | Regular (400) | 表格内容、表单标签、说明文字     |
| 辅助文字 | 12-13px | Regular (400) | 统计卡片标题、面包屑、时间戳     |
| 统计数字 | 26-32px | Bold (700)    | 统计卡片核心数值                 |

### 行高

- 正文行高 1.5 倍字号（14px → 约 21px）
- 标题行高 1.3 倍字号
- 表格行高通过 `padding: 10px 12px` 控制

---

## 5. 间距与布局规范

### 间距系统

基于 4px 基准网格，所有间距为 4 的倍数：

| 场景       | 间距    | 说明                 |
| ---------- | ------- | -------------------- |
| 页面边距   | 20-24px | 内容区到窗口边缘     |
| 模块间距   | 16-20px | 卡片之间、区块之间   |
| 卡片内边距 | 16-24px | 卡片内容到卡片边缘   |
| 控件间距   | 8-12px  | 同行按钮、表单项之间 |
| 紧凑间距   | 4px     | 图标与文字、标签内部 |

### 布局原则

1. **充足留白**——宁可空，不要挤。空白是最廉价的设计手段
2. **卡片式布局**——每个功能区块是一个白底圆角卡片，浮在灰底上
3. **栅格对齐**——统计卡片等宽分布，图表区域按 1:1 或 2:1 分栏
4. **内容居中**——数据不应全部挤在左上角，利用 stretch 和对齐让内容均匀分布
5. **垂直节奏**——模块之间间距一致（16px），形成稳定的视觉节奏

### 典型页面布局

```
┌─────────────────────────────────────────────┐
│  页面标题 (18px bold)                        │
│  副标题/描述 (13px #8c8c8c)                  │
│                                              │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐       │
│  │统计卡│ │统计卡│ │统计卡│ │统计卡│  ← 16px间距
│  └──────┘ └──────┘ └──────┘ └──────┘       │
│                                              │  ← 16-20px模块间距
│  ┌──────────────────────────────────────┐   │
│  │ 搜索栏 / 筛选条件                      │   │
│ └──────────────────────────────────────┘   │
│                                              │
│  ┌──────────────────────────────────────┐   │
│  │ 数据表格                              │   │
│  │                                      │   │
│  │                                      │   │
│  └──────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

---

## 6. 组件规范

### 6.1 卡片

**视觉**：白底 + 极浅边框 + 圆角，hover 时边框变主色。

```css
/* QSS */
QFrame {
  background: #ffffff;
  border: 1px solid #f0f0f0;
  border-radius: 8px;
}
QFrame:hover {
  border: 1px solid #1677ff;
}
```

**关键约束**：

- 卡片内部嵌套的文本框（QLineEdit / QTextEdit）**不得显示边框**——卡片本身就是容器，内部控件不需要再画边框
- 统计卡片顶部加 3px 色条指示器，颜色与指标语义对应

```css
/* 卡片内文本框去边框 */
QFrame QLineEdit,
QFrame QTextEdit {
  border: none;
  border-bottom: 1px solid #f0f0f0; /* 仅保留下划线 */
  border-radius: 0;
  background: transparent;
}
```

### 6.2 按钮

**三种按钮类型**：

| 类型     | 样式               | 用途                           |
| -------- | ------------------ | ------------------------------ |
| 主按钮   | 蓝底白字 `#1677FF` | 主要操作（提交、保存、新增）   |
| 默认按钮 | 白底灰边框         | 次要操作（取消、重置）         |
| 文字按钮 | 无边框蓝字         | 轻量操作（查看详情、编辑链接） |

```css
/* 主按钮 — 通过动态属性 cssClass 区分 */
QPushButton[cssClass='primary'] {
  background: #1677ff;
  color: #ffffff;
  border: 1px solid #1677ff;
  border-radius: 6px;
  padding: 7px 16px;
  font-weight: 500;
}
QPushButton[cssClass='primary']:hover {
  background: #4096ff;
  border-color: #4096ff;
}
QPushButton[cssClass='primary']:pressed {
  background: #0958d9;
  border-color: #0958d9;
}

/* 危险按钮 */
QPushButton[cssClass='danger'] {
  background: #ff4d4f;
  color: #ffffff;
  border: 1px solid #ff4d4f;
  border-radius: 6px;
}

/* 文字按钮 */
QPushButton[cssClass='text'] {
  background: transparent;
  border: none;
  color: #1677ff;
  padding: 4px 8px;
}
QPushButton[cssClass='text']:hover {
  background: #e6f4ff;
}
```

**C++ 端设置动态属性**：

```cpp
auto* btn = new QPushButton("保存");
btn->setProperty("cssClass", "primary");
btn->style()->unpolish(btn);
btn->style()->polish(btn);
```

### 6.3 输入框

```css
QLineEdit {
  border: 1px solid #d9d9d9;
  border-radius: 6px;
  padding: 6px 12px;
  background: #ffffff;
}
QLineEdit:hover {
  border-color: #4096ff;
}
QLineEdit:focus {
  border-color: #1677ff;
  /* QSS 不支持 box-shadow，通过 QPainter 自绘或接受无阴影 */
}
QLineEdit:disabled {
  background: #f5f5f5;
  color: #bfbfbf;
}
```

### 6.4 表格

**核心改进**：去掉网格线，用行间下划线替代；斑马纹交替；表头浅灰加粗。

```css
QTableWidget {
  background: #ffffff;
  border: 1px solid #e8e8e8;
  border-radius: 8px;
  gridline-color: transparent; /* 去掉网格线 */
  alternate-background-color: #fafafa; /* 斑马纹 */
  selection-background-color: #e6f4ff;
  outline: none;
}
QTableWidget::item {
  padding: 10px 12px;
  border-bottom: 1px solid #f0f0f0; /* 仅保留下划线 */
}
QHeaderView::section {
  background: #fafafa;
  color: #595959;
  font-weight: 600;
  font-size: 13px;
  border: none;
  border-bottom: 1px solid #e8e8e8;
}
```

**C++ 端设置**：

```cpp
table->setAlternatingRowColors(true);
table->setSelectionBehavior(QAbstractItemView::SelectRows);
table->setShowGrid(false);  // 关闭网格线
table->horizontalHeader()->setStretchLastSection(true);
```

### 6.5 状态标签

圆角小色块，用 QLabel + objectName 实现：

```cpp
auto* tag = new QLabel("待处理");
tag->setObjectName("tagWarning");
tag->setAlignment(Qt::AlignCenter);
tag->setFixedHeight(24);
tag->setMinimumWidth(60);
```

```css
/* 待处理 — 橙色 */
QLabel#tagWarning {
  background: #fff7e6;
  color: #d46b08;
  border-radius: 4px;
  padding: 2px 8px;
  font-size: 12px;
  font-weight: 500;
}
/* 已完成 — 绿色 */
QLabel#tagSuccess {
  background: #f6ffed;
  color: #389e0d;
  border-radius: 4px;
  padding: 2px 8px;
  font-size: 12px;
  font-weight: 500;
}
/* 逾期/异常 — 红色 */
QLabel#tagError {
  background: #fff2f0;
  color: #cf1322;
  border-radius: 4px;
  padding: 2px 8px;
  font-size: 12px;
  font-weight: 500;
}
/* 进行中 — 蓝色 */
QLabel#tagProcessing {
  background: #e6f4ff;
  color: #1677ff;
  border-radius: 4px;
  padding: 2px 8px;
  font-size: 12px;
  font-weight: 500;
}
```

### 6.6 侧边栏

**核心改进**：手风琴模式——同时只展开一个分组，减少侧边栏长度。

```cpp
// 在 buildSidebar() 中，连接所有 groupHeader 的 toggled 信号
// 当一个分组展开时，折叠其他所有分组
connect(groupHeader, &QPushButton::toggled, this, [this, groupHeader, childContainer](bool checked) {
    if (checked) {
        // 折叠其他所有分组
        for (auto* other : m_groupHeaders) {
            if (other != groupHeader && other->isChecked()) {
                other->setChecked(false);
            }
        }
    }
    childContainer->setVisible(checked);
});
```

**侧边栏样式**：

```css
QWidget {
  background: #001529;
}
QPushButton {
  background: transparent;
  color: rgba(255, 255, 255, 0.7);
  border: none;
  padding: 12px 20px;
  text-align: left;
  font-size: 14px;
  min-height: 40px;
  border-radius: 0;
}
QPushButton:hover {
  background: rgba(255, 255, 255, 0.06);
  color: #ffffff;
}
QPushButton[active='true'] {
  background: #1677ff;
  color: #ffffff;
  font-weight: 600;
}
```

---

## 7. 数据可视化规范

### 7.1 饼图

**当前问题**：饼图没有数据标签，没有悬浮交互，用户无法直观获取数值。

**改进方案**：

```cpp
// 1. 显示百分比标签
auto* series = new QPieSeries();
// ... 添加数据 ...
for (auto* slice : series->slices()) {
    double pct = slice->percentage() * 100;
    slice->setLabel(QString("%1 %2%").arg(slice->label()).arg(pct, 0, 'f', 1));
    slice->setLabelVisible(true);
    slice->setLabelArmLengthFactor(0.15);  // 引线长度
}

// 2. 悬浮显示详情 tooltip
connect(series, &QPieSeries::hovered, this, [](QPieSlice* slice, bool hovered) {
    if (hovered) {
        slice->setExploded(true);
        slice->setLabelFont(QFont("Microsoft YaHei UI", 11, QFont::Bold));
        QToolTip::showText(QCursor::pos(),
            QString("%1\n数量: %2\n占比: %3%")
                .arg(slice->label())
                .arg(slice->value())
                .arg(slice->percentage() * 100, 0, 'f', 1));
    } else {
        slice->setExploded(false);
        slice->setLabelFont(QFont("Microsoft YaHei UI", 9));
        QToolTip::hideText();
    }
});

// 3. 配色方案 — 使用 Ant Design 色板
QColor pieColors[] = {
    QColor("#1677ff"), QColor("#52c41a"), QColor("#fa8c16"),
    QColor("#ff4d4f"), QColor("#722ed1"), QColor("#13c2c2"),
    QColor("#eb2f96"), QColor("#faad14")
};
int ci = 0;
for (auto* slice : series->slices()) {
    slice->setColor(pieColors[ci % 8]);
    slice->setBorderColor(Qt::white);
    slice->setBorderWidth(2);
    ci++;
}
```

### 7.2 柱状图

```cpp
auto* barSet = new QBarSet("工单数");
barSet->setColor(QColor("#1677ff"));
barSet->setBorderColor(QColor("#1677ff"));

// 悬浮高亮 — QBarSeries 的 hovered 信号
connect(barSeries, &QBarSeries::hovered, this, [](bool hovered, int index, QBarSet* set) {
    if (hovered) {
        set->setColor(QColor("#4096ff"));  // hover 变浅
        QToolTip::showText(QCursor::pos(),
            QString("%1: %2").arg(set->label()).arg(set->at(index)));
    } else {
        set->setColor(QColor("#1677ff"));
        QToolTip::hideText();
    }
});
```

### 7.3 趋势线

```cpp
auto* series = new QLineSeries();
// ... 添加数据点 ...
series->setPointsVisible(true);       // 显示数据点
series->setPointLabelsVisible(true);  // 关键数据点标注
series->setPointLabelsFormat("@yPoint");
series->setPointLabelsColor(QColor("#1677ff"));
series->setPointLabelsFont(QFont("Segoe UI", 10));

// 平滑曲线
auto* splineSeries = new QSplineSeries();  // 使用 QSplineSeries 替代 QLineSeries
```

### 7.4 通用图表规范

```cpp
// 所有图表必须设置
chart->setAnimationOptions(QChart::SeriesAnimations);  // 动画过渡
chart->setBackgroundVisible(false);                      // 透明背景
chart->layout()->setContentsMargins(8, 8, 8, 8);        // 内边距
chart->legend()->setAlignment(Qt::AlignBottom);          // 图例底部

// ChartView
chartView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
chartView->setMinimumHeight(280);                   // 统一最小高度
```

---

## 8. 微交互规范

### 8.1 按钮 hover/click

已在 QSS 中通过 `:hover` / `:pressed` 伪状态实现颜色微变。无需额外代码。

### 8.2 卡片 hover 边框变色

```css
QFrame {
  border: 1px solid #f0f0f0;
}
QFrame:hover {
  border: 1px solid #1677ff;
}
```

### 8.3 页面切换过渡

```cpp
// 使用 QPropertyAnimation 实现淡入效果
auto* page = m_contentStack->currentWidget();
auto* anim = new QPropertyAnimation(page, "windowOpacity");
anim->setDuration(150);
anim->setStartValue(0.0);
anim->setEndValue(1.0);
anim->start(QAbstractAnimation::DeleteWhenStopped);
```

> 注意：QStackedWidget 的页面切换默认无动画，需要自定义实现。对于 QWidget 的 opacity 动画，需要设置 `WA_TranslucentBackground` 属性。如果实现复杂度太高，可暂时跳过，优先保证静态视觉质量。

### 8.4 操作后轻提示（Toast）

Qt 没有内置 Toast 组件，需自行实现：

```cpp
// 简易 Toast 实现
class ToastLabel : public QLabel {
    Q_OBJECT
public:
    static void show(const QString& text, QWidget* parent, int duration = 2000) {
        auto* toast = new ToastLabel(text, parent);
        toast->show();
        QTimer::singleShot(duration, toast, &ToastLabel::fadeOut);
    }

private:
    ToastLabel(const QString& text, QWidget* parent)
        : QLabel(text, parent) {
        setAlignment(Qt::AlignCenter);
        setFixedSize(text.length() * 16 + 32, 40);
        setStyleSheet(R"(
            background: rgba(0, 0, 0, 0.75);
            color: #ffffff; border-radius: 6px;
            font-size: 14px; padding: 8px 16px;
        )");
        // 居中定位
        if (parent) {
            move((parent->width() - width()) / 2,
                 (parent->height() - height()) / 2);
        }
    }
    void fadeOut() {
        auto* anim = new QPropertyAnimation(this, "windowOpacity");
        anim->setDuration(300);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        connect(anim, &QPropertyAnimation::finished, this, &QObject::deleteLater);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
};
```

### 8.5 加载等待动画

```cpp
// 使用 QProgressIndicator 或自定义旋转动画
auto* spinner = new QLabel(page);
spinner->setMovie(new QMovie(":/icons/loading.gif"));  // 需要提供 gif 资源
spinner->movie()->start();
```

更轻量的方案——使用 `QTimer` 驱动旋转文字：

```cpp
auto* loadingLabel = new QLabel("加载中...", page);
loadingLabel->setAlignment(Qt::AlignCenter);
loadingLabel->setStyleSheet("color: #8c8c8c; font-size: 14px;");
```

---

## 9. 当前项目问题与改进方向

| #   | 当前问题                                                                 | 改进方案                                                                                     | 涉及规范章节 |
| --- | ------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------- | ------------ |
| 1   | **卡片内文本框有浅灰色边框**，视觉上像嵌套了框，显得杂乱                 | 卡片内 QLineEdit/QTextEdit 去掉边框，仅保留下划线或无边框                                    | §6.1         |
| 2   | **侧边栏过长**，所有分组同时展开导致需要滚动                             | 手风琴模式：同时只展开一个分组，点击展开另一个时自动折叠前一个                               | §6.6         |
| 3   | **饼图缺乏数据标签和悬浮交互**，用户无法直观获取数值                     | 显示百分比标签 + hovered 信号实现 tooltip + 悬浮爆炸效果                                     | §7.1         |
| 4   | **页面有空白区域，数据集中在左上角**                                     | 统计卡片均匀分布（stretch 均分宽度）；表格设置 `stretchLastSection`；内容区使用 stretch 填充 | §5           |
| 5   | **缺乏统一的视觉语言**，各页面样式零散                                   | 全局 QSS 集中管理在 `style.qss`；页面内联样式仅用于特殊场景；统一使用 objectName 和动态属性  | §10          |
| 6   | **页面朴素简陋，缺乏设计感**                                             | 全局背景 `#F0F2F5` + 白底卡片 + 充足间距 + 状态标签色块 + 图表配色                           | §3, §5, §6   |
| 7   | **统计卡片样式不统一**，Dashboard 和综合看板分别定义了 createCard/mkCard | 提取为统一的 `createStatCard()` 工具方法，全局复用                                           | §6.1         |
| 8   | **表格无搜索/筛选联动**，搜索框与数据刷新未连接                          | 搜索框 textChanged 信号连接到数据刷新函数                                                    | §6.4         |
| 9   | **图表无动画过渡**                                                       | 设置 `QChart::SeriesAnimations`                                                              | §7.4         |
| 10  | **操作后无轻提示**                                                       | 实现 ToastLabel 组件，增删改操作后显示                                                       | §8.4         |

---

## 10. Qt/QSS 实现要点

### 10.1 全局 QSS 集中管理

所有通用样式集中在 `resources/style.qss`，通过 `qApp->setStyleSheet()` 全局加载。页面内联样式仅用于该页面独有的样式（如统计卡片的动态颜色）。

**禁止**在多处重复定义相同的样式。如果多个页面都需要统计卡片样式，提取为 QSS 中的通用选择器。

### 10.2 使用 objectName 选择器精准控制

```css
/* 好 — 通过 objectName 精准控制 */
QLabel#tagWarning {
  background: #fff7e6;
  color: #d46b08;
}

/* 差 — 全局影响所有 QLabel */
QLabel {
  background: #fff7e6;
}
```

### 10.3 动态属性驱动状态变化

QSS 支持通过自定义属性选择器：

```css
/* 按钮类型 */
QPushButton[cssClass='primary'] {
  background: #1677ff;
  color: #ffffff;
}
QPushButton[cssClass='danger'] {
  background: #ff4d4f;
  color: #ffffff;
}
QPushButton[cssClass='text'] {
  background: transparent;
  color: #1677ff;
  border: none;
}

/* 侧边栏活跃态 */
QPushButton[active='true'] {
  background: #1677ff;
  color: #ffffff;
}
```

C++ 端修改属性后必须刷新样式：

```cpp
btn->setProperty("cssClass", "primary");
btn->style()->unpolish(btn);
btn->style()->polish(btn);
```

### 10.4 QChart 的 hovered 信号实现 tooltip

```cpp
// 饼图
connect(pieSeries, &QPieSeries::hovered,
    [](QPieSlice* slice, bool hovered) {
        if (hovered) {
            slice->setExploded(true);
            QToolTip::showText(QCursor::pos(),
                QString("%1: %2 (%3%)")
                    .arg(slice->label())
                    .arg(slice->value())
                    .arg(slice->percentage() * 100, 0, 'f', 1));
        } else {
            slice->setExploded(false);
            QToolTip::hideText();
        }
    });

// 柱状图
connect(barSeries, &QBarSeries::hovered,
    [](bool hovered, int index, QBarSet* set) {
        if (hovered) {
            QToolTip::showText(QCursor::pos(),
                QString("%1: %2").arg(set->label()).arg(set->at(index)));
        } else {
            QToolTip::hideText();
        }
    });
```

### 10.5 QPropertyAnimation 实现过渡动画

```cpp
// 淡入
auto* fadeIn = new QPropertyAnimation(widget, "windowOpacity");
fadeIn->setDuration(150);
fadeIn->setStartValue(0.0);
fadeIn->setEndValue(1.0);
fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

// 几何变化（如侧边栏折叠）
auto* slide = new QPropertyAnimation(sidebar, "geometry");
slide->setDuration(200);
slide->setStartValue(sidebar->geometry());
slide->setEndValue(QRect(0, 0, 60, sidebar->height()));  // 折叠到 60px
slide->start(QAbstractAnimation::DeleteWhenStopped);
```

### 10.6 QSS 不支持的特性及替代方案

| 需求                          | QSS 是否支持 | 替代方案                            |
| ----------------------------- | ------------ | ----------------------------------- |
| `box-shadow`                  | 不支持       | 自定义 QWidget::paintEvent 绘制阴影 |
| `transition`                  | 不支持       | QPropertyAnimation                  |
| `linear-gradient` 背景        | 部分支持     | `background: qlineargradient(...)`  |
| `rgba()` 颜色                 | 支持         | `rgba(0, 0, 0, 0.75)`               |
| `::before` / `::after` 伪元素 | 不支持       | 用额外的 QWidget/QLabel 模拟        |

### 10.7 阴影效果的实现

QSS 无法实现 `box-shadow`，但可以通过 `QGraphicsDropShadowEffect` 实现：

```cpp
auto* card = new QFrame();
auto* shadow = new QGraphicsDropShadowEffect(card);
shadow->setOffset(0, 2);
shadow->setBlurRadius(8);
shadow->setColor(QColor(0, 0, 0, 30));  // rgba(0,0,0,0.12)
card->setGraphicsEffect(shadow);
```

> 注意：`QGraphicsDropShadowEffect` 会影响渲染性能，大量卡片时慎用。对于表格等复杂控件不建议使用。卡片边框 `#f0f0f0` 已足够区分层级，阴影为锦上添花。

---

## 附录：快速参考卡片

```
┌─────────────────────────────────────────────┐
│  色值速查                                    │
│  主色 #1677FF  成功 #52C41A  警告 #FA8C16    │
│  危险 #FF4D4F  紫色 #722ED1  青色 #13C2C2    │
│  背景 #F0F2F5  卡片 #FFFFFF  边框 #F0F0F0    │
│  正文 #262626  次文 #8C8C8C  占位 #BFBFBF    │
├─────────────────────────────────────────────┤
│  间距速查                                    │
│  页面边距 20-24px  模块间距 16-20px          │
│  卡片内边距 16-24px  控件间距 8-12px         │
├─────────────────────────────────────────────┤
│  字号速查                                    │
│  页面标题 18-20px bold  模块标题 16px bold   │
│  正文 14px  辅助 12-13px  统计数字 26-32px   │
├─────────────────────────────────────────────┤
│  组件速查                                    │
│  圆角 8px  按钮圆角 6px  输入框圆角 6px      │
│  卡片边框 1px #f0f0f0  表格去网格线          │
│  侧边栏 #001529  手风琴模式                  │
└─────────────────────────────────────────────┘
```

结合我们之前的探讨，我将这个“智慧社区管理平台”Qt 项目的 UI 设计思路与工程落地方案进行了全面、系统的总结。

整体思路可以概括为：**以“现代轻量化办公风”为视觉基调，通过“克制+点缀+微交互”的设计法则提升体验，并依托严谨的 Qt 6 工程架构保障性能与可扩展性。**

以下是完整的设计思路总结：

### 一、 核心设计理念：拿捏“度”的艺术

作为面向社区工作人员和物业的 B 端办公系统，UI 必须传递“专业、靠谱、高效”的属性。

- **风格定位**：现代轻量化办公风（对标 Ant Design Pro / 飞书）。
- **核心法则**：**“克制基调 + 精准点缀 + 微交互”**。坚决摒弃传统 Qt 默认样式的“老气单调”，同时避免过度设计导致的“花哨刺眼”，在两者间寻找完美平衡。

### 二、 视觉与布局策略：用“留白”撑起高级感

不依赖复杂的装饰，而是通过空间关系和色彩层级来构建现代感。

1.  **卡片式布局与呼吸感**
    - **底色与卡片**：全局采用柔和浅灰底色（如 `#F0F2F5`），所有功能区块（统计、表格、表单）均封装在纯白色卡片中。
    - **细节打磨**：卡片采用 8px 圆角，辅以极浅的灰色边框（如 `#F0F0F0`）替代生硬的阴影（规避 Qt 阴影性能问题），通过“灰底白卡”的对比拉开界面层次。
2.  **色彩管理：大面积克制，小面积精准**
    - **基础色**：深灰、浅灰等中性色占据 90% 以上的面积（背景、文字、边框）。
    - **功能色点缀**：主色（蓝）、成功（绿）、警告（橙）仅用于视觉焦点。例如：统计卡片顶部的 3px 彩色指示条、核心大数字、以及“浅底深字”的圆角状态标签（如待处理、已完成）。
3.  **导航系统：深色手风琴侧边栏**
    - 采用深色（如 `#001529`）侧边栏压住阵脚，菜单采用**手风琴模式**（点击展开当前组，自动折叠其他组），避免长菜单带来的视觉混乱。

### 三、 组件细节与微交互：去掉“传统 Qt 味”

在用户高频接触的核心组件上花心思，让界面“活”起来。

1.  **数据表格“减负”**
    - 坚决关闭默认网格线，改用**斑马纹**交替色和极浅的行下划线。
    - 表头采用浅灰底色加粗，整体视觉更轻盈、透气。
2.  **输入控件“弱化”**
    - 弱化输入框边框（如仅保留底边下划线或极浅边框），仅在鼠标聚焦（Focus）时边框变为主色蓝，减少视觉噪音。
3.  **图表交互“灵动”**
    - 饼图/柱状图必须支持 Hover 交互：鼠标悬浮时触发高亮（如饼图切片微微“爆炸”突出），并配合 `QToolTip` 展示精准数据，避免死板的静态图表。
4.  **状态反馈“丝滑”**
    - 摒弃生硬阻塞的 `QMessageBox`，采用 **Toast 轻提示**（半透明黑底白字，2秒后自动淡出），不打断用户心流。
5.  **克制的微动效**
    - 充分利用 QSS 伪状态（`:hover`, `:pressed`），实现按钮悬浮变浅、按压缩沉、卡片悬浮边框高亮等符合直觉的物理反馈。

### 四、 Qt 6 工程落地与避坑指南（深水区方案）

设计再好，也需要扎实的工程实现来兜底。针对 Qt 做现代 UI 的常见痛点，确立了以下技术规范：

1.  **全局样式与动态属性管理**
    - **严禁代码内联**：禁止在业务代码中散落 `setStyleSheet`，所有样式收敛至全局 `style.qss`。
    - **动态属性驱动**：通过 `setProperty("cssClass", "primary")` 结合 QSS 属性选择器复用样式。
    - **强制刷新机制**：封装轻量级的 `StyleHelper::refreshWidget()` 工具函数（内含 `unpolish` -> `polish` -> `update`），解决动态属性修改后样式不生效的顽疾。
2.  **Toast 提示的防坑处理**
    - **防抢焦点**：设置 `Qt::WA_ShowWithoutActivating` 和 `Qt::NoFocus`，确保弹窗不干扰底层输入。
    - **防黑边闪烁**：摒弃原生的 `windowOpacity` 动画，改用 `QGraphicsOpacityEffect` 控制透明度，彻底解决 Windows 下的渲染 Artifact。
3.  **复杂表格的性能保障**
    - **抛弃野路子**：严禁使用 `setCellWidget` 塞入操作按钮（会导致大数据量滚动卡顿）。
    - **正统方案**：采用 `QTableView` + `QStyledItemDelegate` 重绘操作列和状态开关，并将 Delegate 封装为通用模板，兼顾极致性能与代码复用。
4.  **图表 Hover 的遮挡规避**
    - 开启 `setMouseTracking(true)`，使用全局坐标 `QCursor::pos()` 定位 `QToolTip`，并合理设置饼图引线长度（`setLabelArmLengthFactor`），防止文字堆叠和提示框被遮挡。

### 五、 架构扩展性：面向未来的“暗黑模式”

在架构设计之初就为未来的主题切换（如暗黑模式）留好接口：

- **全局主题属性**：在 `qApp` 层面设置动态属性（如 `theme="dark"`）。
- **QSS 属性选择器**：在样式表中通过 `QWidget[theme="dark"]` 预设暗黑配色。
- **信号驱动刷新**：切换主题时，通过 `ThemeManager` 发射 `themeChanged` 信号，遍历并刷新所有顶层窗口，配合 `QPalette` 确保原生控件颜色完美反转。

---

**一句话总结：**
这套设计思路就是**用 Ant Design 的“卡片留白与色彩克制”保底（不花哨），用“微交互与细节打磨”提神（不乏味），最后用“Delegate、Effect 和全局 QSS 架构”护航（不卡顿、好维护）**，打造出一个既有颜值又有战斗力的现代 Qt 桌面应用。
