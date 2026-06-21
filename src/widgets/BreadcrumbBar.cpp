#include "BreadcrumbBar.h"
#include "PagesCommon.h"
#include <QMouseEvent>

BreadcrumbBar::BreadcrumbBar(QWidget *parent) : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(8);
    m_layout->addStretch();
    setStyleSheet("background: transparent;");
}

void BreadcrumbBar::registerModuleTarget(const QString &module, const QString &pageKey)
{
    if (!module.isEmpty() && !pageKey.isEmpty())
        m_moduleTargets[module] = pageKey;
}

void BreadcrumbBar::setPath(const QString &module, const QString &page)
{
    QLayoutItem *item;
    while ((item = m_layout->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    // 首页
    auto *homeLabel = new QLabel(QStringLiteral("首页"), this);
    homeLabel->setStyleSheet(
        "QLabel { color: #6B6B6B; font-size: 13px; background: transparent; padding: 2px 4px; }"
        "QLabel:hover { color: #92400E; text-decoration: underline; }");
    homeLabel->setCursor(Qt::PointingHandCursor);
    homeLabel->setProperty("breadcrumbTarget", m_homeTarget);
    homeLabel->installEventFilter(this);
    m_layout->addWidget(homeLabel);

    if (!module.isEmpty())
    {
        auto *sep1 = new QLabel(QStringLiteral("/"), this);
        sep1->setStyleSheet("color: #D4D0C8; font-size: 13px; background: transparent;");
        m_layout->addWidget(sep1);

        auto *moduleLabel = new QLabel(module, this);
        moduleLabel->setStyleSheet(
            "QLabel { color: #6B6B6B; font-size: 13px; background: transparent; padding: 2px 4px; }"
            "QLabel:hover { color: #92400E; text-decoration: underline; }");
        moduleLabel->setCursor(Qt::PointingHandCursor);
        moduleLabel->setProperty("breadcrumbModule", module);
        QString target = m_moduleTargets.value(module);
        if (!target.isEmpty())
        {
            moduleLabel->setProperty("breadcrumbTarget", target);
            moduleLabel->installEventFilter(this);
        }
        m_layout->addWidget(moduleLabel);
    }

    auto *sep2 = new QLabel(QStringLiteral("/"), this);
    sep2->setStyleSheet("color: #D4D0C8; font-size: 13px; background: transparent;");
    m_layout->addWidget(sep2);

    auto *pageLabel = new QLabel(page, this);
    pageLabel->setStyleSheet(
        "QLabel { color: #141413; font-size: 13px; font-weight: 600; background: transparent; padding: 2px 4px; }");
    m_layout->addWidget(pageLabel);

    m_layout->addStretch();
}

bool BreadcrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            auto *w = qobject_cast<QWidget *>(watched);
            if (!w)
                return QWidget::eventFilter(watched, event);
            QString target = w->property("breadcrumbTarget").toString();
            if (!target.isEmpty())
            {
                QString module = w->property("breadcrumbModule").toString();
                if (!module.isEmpty())
                    emit moduleClicked(module);
                emit itemClicked(target);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
