# Qt 样式与布局问题修复记录

## 1. QComboBox/QLineEdit 背景变黑/透明

**现象**: 下拉框、输入框背景色异常（黑色或透明），看不清文字。

**根因**: Qt 样式表的级联机制。在父控件上使用 `setStyleSheet()` 设置的样式会**级联到所有子控件**，覆盖全局样式表。

### 错误写法

```cpp
// 方式1: QWidget{} 选择器匹配所有子控件
toolbar->setStyleSheet("QWidget{background:transparent;}");
// 结果: toolbar 内的 QComboBox、QLineEdit 全部变透明

// 方式2: 裸属性声明也会级联
toolbar->setStyleSheet("background:transparent;");
// 结果: 同上，子控件背景被覆盖
```

### 正确写法

```cpp
// 使用 #objectName 选择器，只作用于控件自身
toolbar->setObjectName("filterToolbar");
toolbar->setStyleSheet("#filterToolbar{background:transparent;}");
// 结果: 只有 toolbar 自身透明，子控件使用全局样式表的 background:#ffffff
```

**原则**: 需要给容器设置透明背景时，**必须用 `#objectName` 选择器限定作用范围**，避免级联到子控件。

---

## 2. 表格操作按钮超出单元格边框

**现象**: 表格最后一列（操作列）的按钮超出单元格右边界。

**根因**: `setCellWidget()` 创建的 QWidget 不受单元格宽度约束。QPushButton 的 `minimumSizeHint()` 可能超出列宽，导致 widget 溢出单元格边界。

### 错误方案

```cpp
// setCellWidget 的 widget 尺寸由自身布局决定，不受 cell 约束
auto* actionWidget = new QWidget(table);
auto* actionLayout = new QHBoxLayout(actionWidget);
auto* btn = new QPushButton("受理");
actionLayout->addWidget(btn);
table->setCellWidget(row, col, actionWidget);
// 问题: btn 的 minimumSizeHint 可能超出列宽
```

### 正确方案

```cpp
// 使用 QTableWidgetItem 文本项 + cellClicked 信号
auto* actionItem = new QTableWidgetItem("受理");
actionItem->setTextAlignment(Qt::AlignCenter);
actionItem->setForeground(QColor("#1677ff"));
actionItem->setData(Qt::UserRole, recordId);      // 存储记录ID
actionItem->setData(Qt::UserRole + 1, status);     // 存储状态
table->setItem(row, col, actionItem);

// 信号处理
connect(table, &QTableWidget::cellClicked, this, [=](int r, int c) {
    if (c != col) return;
    auto* item = table->item(r, c);
    if (!item) return;
    qint64 id = item->data(Qt::UserRole).toLongLong();
    // 执行操作...
});

// 鼠标手型
connect(table, &QTableWidget::cellEntered, this, [=](int r, int c) {
    table->viewport()->setCursor(c == col ? Qt::PointingHandCursor : Qt::ArrowCursor);
});
```

**原则**: 操作列**不要用 `setCellWidget`**，用 `QTableWidgetItem` 文本永远在单元格内。

---

## 3. Lambda 悬空引用导致闪退

**现象**: 筛选/搜索操作时应用闪退，无报错信息。

**根因**: `std::function<void()> loadXxx = [&]()` 捕获了局部指针变量的**引用**。函数返回后局部变量被销毁，引用变成悬空引用。

### 错误写法

```cpp
void createPage() {
    auto* table = new QTableWidget();
    auto* searchEdit = new QLineEdit();
    std::function<void()> loadData = [&]() {  // [&] 捕获引用!
        searchEdit->text();  // 悬空引用! 函数返回后 searchEdit 变量已销毁
    };
    connect(searchEdit, &QLineEdit::textChanged, [=]() { loadData(); });
    // 当信号触发时, loadData 内的 &searchEdit 已是悬空引用
}
```

### 正确写法

```cpp
void createPage() {
    auto* table = new QTableWidget();
    auto* searchEdit = new QLineEdit();
    std::function<void()> loadData = [table, searchEdit]() {  // 按值捕获指针!
        searchEdit->text();  // OK: searchEdit 是指针值的拷贝, 指向的堆对象仍有效
    };
    connect(searchEdit, &QLineEdit::textChanged, [=]() { loadData(); });
}
```

**原则**: lambda 如果会在作用域外被调用（如信号连接），**必须按值捕获指针**，不能用 `[&]`。

---

## 4. Qt 框架警告过滤

**现象**: 控制台输出 `Unknown property box-shadow` 或 `QFont::setPointSize: Point size <= 0 (-1)` 警告。

**根因**: Qt 框架内部 QSS 解析器的已知问题，非项目代码引起。

**修复**: 在 `main.cpp` 中安装自定义消息处理器过滤这些警告：

```cpp
static QtMessageHandler originalHandler = nullptr;
static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (type == QtWarningMsg) {
        if (msg.contains("Unknown property box-shadow")) return;
        if (msg.contains("QFont::setPointSize: Point size <= 0")) return;
    }
    if (originalHandler) originalHandler(type, context, msg);
}
```

---

## 5. SQL 字段名与数据库 Schema 不匹配

**现象**: 页面无数据或查询静默失败。

**根因**: C++ 代码中使用的字段名与 `schema.sql` 中定义的字段名不一致。

**案例**:
- 代码写 `h.building_no` → 实际字段名 `h.house_code`
- 代码写 `h.room_no` → 实际字段名 `h.room_number`
- 代码写 `ps.space_no` → 实际字段名 `ps.space_code`
- 代码用整数筛选 VARCHAR 字段: `AND category = 1` → 应为 `AND category = '环境'`

**原则**: 修改查询前**先对照 `sql/schema.sql` 确认字段名和类型**。
