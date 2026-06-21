#include "pages/PageFactory.h"

#include <QMap>
#include <functional>

namespace PageFactory {

BasePage *createPage(const QString &key)
{
    static const QMap<QString, std::function<BasePage*()>> registry = {
        // Dashboard & 个人中心
        {QStringLiteral("1"),   []() { return createDashboardPage(); }},
        {QStringLiteral("102"), []() { return createTodoPage(); }},
        {QStringLiteral("103"), []() { return createMessagePage(); }},

        // 基础档案
        {QStringLiteral("201"), []() { return createArchivePage(QStringLiteral("org")); }},
        {QStringLiteral("202"), []() { return createArchivePage(QStringLiteral("estate")); }},
        {QStringLiteral("203"), []() { return createArchivePage(QStringLiteral("house")); }},
        {QStringLiteral("204"), []() { return createArchivePage(QStringLiteral("resident")); }},
        {QStringLiteral("205"), []() { return createArchivePage(QStringLiteral("vehicle")); }},
        {QStringLiteral("206"), []() { return createArchivePage(QStringLiteral("facility")); }},
        {QStringLiteral("207"), []() { return createArchivePage(QStringLiteral("grid")); }},
        {QStringLiteral("208"), []() { return createArchivePage(QStringLiteral("special")); }},

        // 小区管理
        {QStringLiteral("301"), []() { return createPropertyPage(QStringLiteral("workorder")); }},
        {QStringLiteral("302"), []() { return createPropertyPage(QStringLiteral("complaint")); }},
        {QStringLiteral("303"), []() { return createPropertyPage(QStringLiteral("inspection")); }},
        {QStringLiteral("304"), []() { return createPropertyPage(QStringLiteral("announcement")); }},
        {QStringLiteral("305"), []() { return createPropertyPage(QStringLiteral("visitor")); }},
        {QStringLiteral("306"), []() { return createPropertyPage(QStringLiteral("topic")); }},
        {QStringLiteral("307"), []() { return createPropertyPage(QStringLiteral("parking")); }},
        {QStringLiteral("308"), []() { return createPropertyPage(QStringLiteral("billing")); }},
        {QStringLiteral("309"), []() { return createPropertyPage(QStringLiteral("income")); }},

        // 社区治理
        {QStringLiteral("401"), []() { return createGovernancePage(QStringLiteral("event")); }},
        {QStringLiteral("402"), []() { return createGovernancePage(QStringLiteral("inspection")); }},
        {QStringLiteral("403"), []() { return createGovernancePage(QStringLiteral("care")); }},
        {QStringLiteral("404"), []() { return createGovernancePage(QStringLiteral("supervision")); }},
        {QStringLiteral("405"), []() { return createGovernancePage(QStringLiteral("opinion")); }},
        {QStringLiteral("406"), []() { return createGovernancePage(QStringLiteral("assessment")); }},

        // 社区服务
        {QStringLiteral("501"), []() { return createServicePage(QStringLiteral("volunteer")); }},
        {QStringLiteral("502"), []() { return createServicePage(QStringLiteral("convenience")); }},
        {QStringLiteral("503"), []() { return createServicePage(QStringLiteral("job")); }},

        // 统计分析
        {QStringLiteral("601"), []() { return createReportPage(QStringLiteral("workorder")); }},
        {QStringLiteral("602"), []() { return createReportPage(QStringLiteral("event")); }},
        {QStringLiteral("603"), []() { return createReportPage(QStringLiteral("service")); }},
        {QStringLiteral("604"), []() { return createReportPage(QStringLiteral("dashboard")); }},

        // 系统管理
        {QStringLiteral("701"), []() { return createSystemPage(QStringLiteral("user")); }},
        {QStringLiteral("702"), []() { return createSystemPage(QStringLiteral("role")); }},
        {QStringLiteral("703"), []() { return createSystemPage(QStringLiteral("menu")); }},
        {QStringLiteral("704"), []() { return createSystemPage(QStringLiteral("dict")); }},
        {QStringLiteral("705"), []() { return createSystemPage(QStringLiteral("log")); }},
        {QStringLiteral("706"), []() { return createSystemPage(QStringLiteral("ai")); }},
    };

    auto it = registry.find(key);
    return it != registry.end() ? it.value()() : nullptr;
}

} // namespace PageFactory
