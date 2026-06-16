#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include "database/DatabaseManager.h"
#include "services/AuthService.h"
#include "services/DemoDataService.h"
#include "widgets/LoginWidget.h"
#include "widgets/MainWindow.h"

// 过滤 Qt QSS 解析器的 "Unknown property box-shadow" 警告
// Qt 框架已知警告过滤：
// 1. box-shadow: QSS 中 border-radius + background 触发内部解析 box-shadow
// 2. setPointSize <= 0: QSS 解析 font-size 时对默认字体(pointSize=-1)调用 setPointSize
static QtMessageHandler originalHandler = nullptr;
static void customMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (type == QtWarningMsg) {
        if (msg.contains("Unknown property box-shadow")) return;
        if (msg.contains("QFont::setPointSize: Point size <= 0")) return;
    }
    if (originalHandler) {
        originalHandler(type, context, msg);
    }
}

int main(int argc, char* argv[]) {
    // 安装消息过滤器
    originalHandler = qInstallMessageHandler(customMessageHandler);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/app.svg"));
    app.setApplicationName(QStringLiteral("智慧社区管理平台"));
    app.setOrganizationName("SmartCommunity");
    app.setApplicationVersion("1.0.0");

    // 设置全局字体
    QFont defaultFont;
    defaultFont.setFamily(QStringLiteral("Microsoft YaHei"));
    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

    // 加载全局样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
        qDebug() << "Global stylesheet loaded.";
    } else {
        qWarning() << "Failed to load style.qss";
    }

    qDebug() << "=== Smart Community Management Platform ===";
    qDebug() << "Initializing...";

    // 初始化数据库
    auto& db = DatabaseManager::instance();
    if (!db.initialize("smart_community.db")) {
        qCritical() << "Failed to initialize database!";
        return -1;
    }

    // 初始化表结构
    db.initSchema();

    // 初始化演示数据（如果数据库为空）
    DemoDataService::initIfEmpty();

    qDebug() << "Database and demo data ready.";

    // 创建登录窗口
    LoginWidget loginWidget;
    MainWindow* mainWindow = nullptr;

    QObject::connect(&loginWidget, &LoginWidget::loginSuccess, [&]() {
        qDebug() << "Login successful, creating main window...";
        if (!mainWindow) {
            mainWindow = new MainWindow();
            QObject::connect(mainWindow, &MainWindow::logoutRequested, [&]() {
                AuthService::instance().logout();
                mainWindow->close();
                delete mainWindow;
                mainWindow = nullptr;
                loginWidget.show();
            });
        }
        loginWidget.hide();
        mainWindow->showMaximized();
    });

    loginWidget.show();

    int result = app.exec();

    // 清理
    if (mainWindow) delete mainWindow;
    db.close();

    qDebug() << "Application exited with code:" << result;
    return result;
}
