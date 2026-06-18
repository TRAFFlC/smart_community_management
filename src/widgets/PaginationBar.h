#ifndef PAGINATIONBAR_H
#define PAGINATIONBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <functional>

class PaginationBar : public QWidget {
    Q_OBJECT
public:
    explicit PaginationBar(QWidget* parent = nullptr)
        : QWidget(parent), m_currentPage(1), m_totalPages(1), m_totalCount(0), m_pageSize(20)
    {
        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(8, 4, 8, 4);
        layout->setSpacing(8);

        m_totalLabel = new QLabel(QStringLiteral("共 0 条"), this);
        m_totalLabel->setStyleSheet("color: #64748b; font-size: 12px;");
        layout->addWidget(m_totalLabel);
        layout->addStretch();

        auto btnStyle = "QPushButton { background: #ffffff; color: #334155; border: 1px solid #e2e8f0; border-radius: 4px; padding: 4px 12px; min-height: 28px; font-size: 13px; }"
                        "QPushButton:hover { border-color: #b45309; color: #b45309; background: #fff7ed; }"
                        "QPushButton:disabled { color: #cbd5e1; border-color: #f1f5f9; background: #f8fafc; }";

        m_prevBtn = new QPushButton(QStringLiteral("上一页"), this);
        m_prevBtn->setStyleSheet(btnStyle);
        m_prevBtn->setEnabled(false);
        layout->addWidget(m_prevBtn);

        m_pageLabel = new QLabel(QStringLiteral("第 1/1 页"), this);
        m_pageLabel->setStyleSheet("color: #475569; font-size: 13px; padding: 0 8px;");
        layout->addWidget(m_pageLabel);

        m_nextBtn = new QPushButton(QStringLiteral("下一页"), this);
        m_nextBtn->setStyleSheet(btnStyle);
        m_nextBtn->setEnabled(false);
        layout->addWidget(m_nextBtn);

        layout->addSpacing(12);

        auto* pageSizeLabel = new QLabel(QStringLiteral("每页"), this);
        pageSizeLabel->setStyleSheet("color: #64748b; font-size: 13px;");
        layout->addWidget(pageSizeLabel);

        m_pageSizeCombo = new QComboBox(this);
        m_pageSizeCombo->addItem(QStringLiteral("20"), 20);
        m_pageSizeCombo->addItem(QStringLiteral("50"), 50);
        m_pageSizeCombo->addItem(QStringLiteral("100"), 100);
        m_pageSizeCombo->setFixedWidth(70);
        m_pageSizeCombo->setStyleSheet(
            "QComboBox { min-height: 28px; padding: 2px 6px; border: 1px solid #e2e8f0; border-radius: 4px; background: #ffffff; color: #334155; }"
            "QComboBox:hover { border-color: #cbd5e1; }"
            "QComboBox:focus { border-color: #b45309; }"
            "QComboBox::drop-down { border: none; width: 20px; }"
            "QComboBox::down-arrow { border-top-color: #64748b; }"
        );
        layout->addWidget(m_pageSizeCombo);

        connect(m_prevBtn, &QPushButton::clicked, this, [this]() {
            if (m_currentPage > 1) { m_currentPage--; emit pageChanged(); }
        });
        connect(m_nextBtn, &QPushButton::clicked, this, [this]() {
            if (m_currentPage < m_totalPages) { m_currentPage++; emit pageChanged(); }
        });
        connect(m_pageSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            m_pageSize = m_pageSizeCombo->currentData().toInt();
            m_currentPage = 1;
            emit pageChanged();
        });
    }

    int pageSize() const { return m_pageSize; }
    int offset() const { return (m_currentPage - 1) * m_pageSize; }
    int currentPage() const { return m_currentPage; }
    int totalPages() const { return m_totalPages; }
    int totalCount() const { return m_totalCount; }

    void setTotalCount(int count) {
        m_totalCount = count;
        m_totalPages = std::max(1, (count + m_pageSize - 1) / m_pageSize);
        if (m_currentPage > m_totalPages) m_currentPage = m_totalPages;
        if (m_currentPage < 1) m_currentPage = 1;
    }

    void refreshData() {
        m_pageLabel->setText(QStringLiteral("第 %1/%2 页").arg(m_currentPage).arg(m_totalPages));
        m_totalLabel->setText(QStringLiteral("共 %1 条").arg(m_totalCount));
        m_prevBtn->setEnabled(m_currentPage > 1);
        m_nextBtn->setEnabled(m_currentPage < m_totalPages);
    }

    void resetPage() {
        m_currentPage = 1;
    }

signals:
    void pageChanged();

private:
    int m_currentPage;
    int m_totalPages;
    int m_totalCount;
    int m_pageSize;
    QLabel* m_totalLabel;
    QLabel* m_pageLabel;
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QComboBox* m_pageSizeCombo;
};

#endif // PAGINATIONBAR_H
