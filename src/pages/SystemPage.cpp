#include "pages/PageFactory.h"
#include "PagesCommon.h"

// ========== System Pages ==========
BasePage *PageFactory::createSystemPage(const QString &sub)
{
    auto *page = new BasePage();
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    auto &db = DatabaseManager::instance();

    // 空状态提示（根据 sub 定制文案）
    QString sysEmptyText;
    if (sub == "user")
        sysEmptyText = QStringLiteral("暂无用户记录");
    else if (sub == "role")
        sysEmptyText = QStringLiteral("暂无角色记录");
    else if (sub == "dict")
        sysEmptyText = QStringLiteral("暂无字典记录");
    else if (sub == "log")
        sysEmptyText = QStringLiteral("暂无日志记录");
    else
        sysEmptyText = QStringLiteral("暂无数据");
    auto *emptyHint = UiKit::createEmptyHintLabel(sysEmptyText, page);

    if (sub == "user") { buildSystemUser(page, layout, db, emptyHint); return page; }
    if (sub == "role") { buildSystemRole(page, layout, db, emptyHint); return page; }
    if (sub == "menu") { buildSystemMenu(page, layout, db, emptyHint); return page; }
    if (sub == "dict") { buildSystemDict(page, layout, db, emptyHint); return page; }
    if (sub == "log") { buildSystemLog(page, layout, db, emptyHint); return page; }
    if (sub == "ai") { buildSystemAI(page, layout, db, emptyHint); return page; }

    // 默认空页面
    layout->addWidget(emptyHint);
    return page;
}
// === END createSystemPage ===
