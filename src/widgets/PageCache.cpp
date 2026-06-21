#include "PageCache.h"
#include "PagesCommon.h"
#include "pages/BasePage.h"
#include "pages/PageFactory.h"

PageCache::PageCache(QStackedWidget *stack, QObject *parent)
    : QObject(parent), m_stack(stack)
{
}

BasePage *PageCache::getOrCreatePage(const QString &key)
{
    auto it = m_pages.constFind(key);
    if (it != m_pages.constEnd())
        return qobject_cast<BasePage *>(it.value());

    BasePage *page = PageFactory::createPage(key);
    if (page)
    {
        m_pages[key] = page;
        if (m_stack)
            m_stack->addWidget(page);
        emit pageCreated(page);
        return page;
    }

    QWidget *placeholder = createPlaceholder();
    m_pages[key] = placeholder;
    if (m_stack)
        m_stack->addWidget(placeholder);
    return nullptr;
}

QWidget *PageCache::pageWidget(const QString &key) const
{
    return m_pages.value(key, nullptr);
}

BasePage *PageCache::refreshPage(const QString &key)
{
    auto it = m_pages.find(key);
    if (it != m_pages.end())
    {
        QWidget *oldPage = it.value();
        if (oldPage && m_stack)
        {
            m_stack->removeWidget(oldPage);
        }
        if (oldPage)
            delete oldPage;  // 同步删除，避免 deleteLater 的悬挂指针风险
        m_pages.erase(it);
    }
    return getOrCreatePage(key);
}

void PageCache::clear()
{
    for (auto it = m_pages.begin(); it != m_pages.end(); ++it)
    {
        QWidget *w = it.value();
        if (w && m_stack)
            m_stack->removeWidget(w);
        if (w)
            w->deleteLater();
    }
    m_pages.clear();
}

QWidget *PageCache::createPlaceholder()
{
    auto *placeholder = new QWidget();
    auto *placeholderLayout = new QVBoxLayout(placeholder);
    placeholderLayout->setContentsMargins(20, 20, 20, 20);
    placeholderLayout->addStretch();
    auto *placeholderLabel = new QLabel(QStringLiteral("功能开发中，敬请期待..."), placeholder);
    placeholderLabel->setStyleSheet("font-size: 18px; color: #6B6B6B; background: transparent;");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLayout->addWidget(placeholderLabel);
    placeholderLayout->addStretch();
    return placeholder;
}
