#ifndef PAGES_COMMON_H
#define PAGES_COMMON_H

// ============================================================================
// 此文件保留为页面构建的常用 Qt 头文件聚合入口。
// 目标是逐步缩小：新文件应直接按需包含 Qt 头文件，不再引用此处。
// 已移除的 rarely-used 头文件（QButtonGroup、QRadioButton、QTreeWidget、
// QFileDialog、QPainter 等）请在使用它们的 .cpp 中直接包含。
// ============================================================================

// Qt 常用布局/容器/控件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTabWidget>
#include <QGroupBox>
#include <QFrame>

// Qt 基础控件
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QDateEdit>

// Qt 表格
#include <QTableWidget>
#include <QHeaderView>

// Qt 对话框
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>

// Qt 应用/菜单/定时器
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QIcon>

// 项目通用头文件（页面高频使用的基础设施）
#include "PaginationBar.h"
#include "database/DatabaseManager.h"
#include "models/Constants.h"
#include "utils/Utils.h"
#include "ui_kit/UiKit.h"
#include "ui_kit/AuthHelpers.h"
#include "ui_kit/DbHelpers.h"

// 标准库
#include <functional>
#include <memory>

#endif // PAGES_COMMON_H
