#include "WorkOrderDetailDialog.h"

#include "../database/DatabaseManager.h"
#include "../models/Constants.h"
#include "../ui_kit/UiKit.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QTextEdit>
#include <QVBoxLayout>

void WorkOrderDetailDialog::show(QWidget* parent, qint64 workOrderId) {
    if (workOrderId <= 0) return;

    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT id, order_no, title, order_type, priority, status, reporter_id, reporter_name, "
              "reporter_phone, description, location_desc, create_time, accept_time, "
              "assign_to, assign_time, finish_time, close_time, result_desc, images "
              "FROM wo_work_order WHERE id = :id AND del_flag = 0");
    q.bindValue(":id", workOrderId);
    if (!q.exec() || !q.next()) {
        QMessageBox::warning(parent, QStringLiteral("提示"), QStringLiteral("工单不存在或已被删除"));
        return;
    }

    QString orderNo = q.value(1).toString();
    QString title = q.value(2).toString();
    int orderType = q.value(3).toInt();
    int priority = q.value(4).toInt();
    int status = q.value(5).toInt();
    qint64 reporterId = q.value(6).toLongLong();
    QString reporterName = q.value(7).toString();
    QString reporterPhone = q.value(8).toString();
    QString description = q.value(9).toString();
    QString locationDesc = q.value(10).toString();
    QDateTime createTime = q.value(11).toDateTime();
    QDateTime acceptTime = q.value(12).toDateTime();
    qint64 assignTo = q.value(13).toLongLong();
    QDateTime assignTime = q.value(14).toDateTime();
    QDateTime finishTime = q.value(15).toDateTime();
    QDateTime closeTime = q.value(16).toDateTime();
    QString resultDesc = q.value(17).toString();
    QString imagesStr = q.value(18).toString();

    QString assigneeName;
    if (assignTo > 0) {
        QSqlQuery assignQ(DatabaseManager::instance().database());
        assignQ.prepare("SELECT nickname, username FROM sys_user WHERE id = :id AND del_flag = 0");
        assignQ.bindValue(":id", assignTo);
        if (assignQ.exec() && assignQ.next()) {
            assigneeName = assignQ.value(0).toString();
            if (assigneeName.isEmpty()) assigneeName = assignQ.value(1).toString();
        }
    }

    QDialog dlg(parent);
    dlg.setWindowTitle(QStringLiteral("工单详情 - %1").arg(orderNo));
    dlg.setMinimumWidth(560);
    auto* dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setContentsMargins(24, 20, 24, 20);
    dlgLayout->setSpacing(16);

    auto* titleLabel = new QLabel(title, &dlg);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #0f172a;");
    titleLabel->setWordWrap(true);
    dlgLayout->addWidget(titleLabel);

    QColor statusColor(WorkOrderStatus::color(status));
    auto* statusTag = new QLabel(WorkOrderStatus::label(status), &dlg);
    statusTag->setStyleSheet(QString("color: %1; background: rgba(%2,%3,%4,0.12); "
        "padding: 4px 12px; border-radius: 4px; font-size: 12px; font-weight: 500;")
        .arg(statusColor.name())
        .arg(statusColor.red()).arg(statusColor.green()).arg(statusColor.blue()));
    dlgLayout->addWidget(statusTag);
    dlgLayout->addSpacing(4);

    // ========== 基本信息区 ==========
    auto* infoTitle = new QLabel(QStringLiteral("基本信息"), &dlg);
    infoTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #0f172a; border-left: 3px solid #b45309; padding-left: 8px;");
    dlgLayout->addWidget(infoTitle);

    auto* infoFrame = new QFrame(&dlg);
    infoFrame->setStyleSheet("QFrame { background: #fafafa; border: 1px solid #e2e8f0; border-radius: 6px; }");
    auto* infoLayout = new QGridLayout(infoFrame);
    infoLayout->setContentsMargins(16, 12, 16, 12);
    infoLayout->setHorizontalSpacing(24);
    infoLayout->setVerticalSpacing(10);

    auto addInfoItem = [infoFrame](QGridLayout* layout, int row, int col, const QString& label, const QString& value) {
        auto* lbl = new QLabel(label, infoFrame);
        lbl->setStyleSheet("color: #64748b; font-size: 12px; background: transparent;");
        layout->addWidget(lbl, row, col * 2);
        auto* val = new QLabel(value.isEmpty() ? QStringLiteral("--") : value, infoFrame);
        val->setStyleSheet("color: #0f172a; font-size: 13px; background: transparent; font-weight: 500;");
        val->setWordWrap(true);
        layout->addWidget(val, row, col * 2 + 1);
    };

    auto fmtTime = [](const QDateTime& dt) -> QString {
        return dt.isValid() ? dt.toString("yyyy-MM-dd hh:mm") : QString();
    };

    addInfoItem(infoLayout, 0, 0, QStringLiteral("工单号"), orderNo);
    addInfoItem(infoLayout, 0, 1, QStringLiteral("类型"), WorkOrderType::label(orderType));
    addInfoItem(infoLayout, 1, 0, QStringLiteral("优先级"), WorkOrderPriority::label(priority));
    addInfoItem(infoLayout, 1, 1, QStringLiteral("状态"), WorkOrderStatus::label(status));
    addInfoItem(infoLayout, 2, 0, QStringLiteral("报修人"), reporterName);
    addInfoItem(infoLayout, 2, 1, QStringLiteral("联系电话"), reporterPhone);
    addInfoItem(infoLayout, 3, 0, QStringLiteral("创建时间"), fmtTime(createTime));
    addInfoItem(infoLayout, 3, 1, QStringLiteral("处理人"), assigneeName);
    addInfoItem(infoLayout, 4, 0, QStringLiteral("完成时间"), fmtTime(finishTime));
    addInfoItem(infoLayout, 4, 1, QStringLiteral("位置"), locationDesc);

    dlgLayout->addWidget(infoFrame);

    // ========== 描述区 ==========
    auto* descTitle = new QLabel(QStringLiteral("工单描述"), &dlg);
    descTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #0f172a; border-left: 3px solid #b45309; padding-left: 8px;");
    dlgLayout->addWidget(descTitle);

    auto* descEdit = new QTextEdit(&dlg);
    descEdit->setReadOnly(true);
    descEdit->setPlainText(description.isEmpty() ? QStringLiteral("（无描述）") : description);
    descEdit->setStyleSheet("QTextEdit { background: #fafafa; border: 1px solid #e2e8f0; border-radius: 4px;"
        " padding: 10px; font-size: 13px; color: #0f172a; }");
    descEdit->setFixedHeight(80);
    dlgLayout->addWidget(descEdit);

    // ========== 附件图片区 ==========
    QStringList imagePaths = imagesStr.split(",", Qt::SkipEmptyParts);
    if (!imagePaths.isEmpty()) {
        auto* imgTitle = new QLabel(QStringLiteral("附件图片"), &dlg);
        imgTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #0f172a; border-left: 3px solid #2563eb; padding-left: 8px;");
        dlgLayout->addWidget(imgTitle);

        auto* imgContainer = new QWidget(&dlg);
        imgContainer->setStyleSheet("background: #fafafa; border: 1px solid #e2e8f0; border-radius: 6px;");
        auto* imgLayout = new QHBoxLayout(imgContainer);
        imgLayout->setContentsMargins(12, 8, 12, 8);
        imgLayout->setSpacing(8);

        for (const QString& path : imagePaths) {
            QString trimmed = path.trimmed();
            if (trimmed.isEmpty()) continue;
            QPixmap pix(trimmed);
            if (!pix.isNull()) {
                auto* imgLabel = new QLabel(imgContainer);
                imgLabel->setFixedSize(80, 80);
                imgLabel->setScaledContents(true);
                imgLabel->setPixmap(pix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                imgLabel->setStyleSheet("border: 1px solid #e2e8f0; border-radius: 4px; background: #ffffff;");
                imgLabel->setToolTip(QFileInfo(trimmed).fileName());
                imgLayout->addWidget(imgLabel);
            } else {
                auto* nameLabel = new QLabel(QFileInfo(trimmed).fileName(), imgContainer);
                nameLabel->setStyleSheet("color: #64748b; font-size: 12px; background: transparent; padding: 4px 8px;");
                imgLayout->addWidget(nameLabel);
            }
        }
        imgLayout->addStretch();
        dlgLayout->addWidget(imgContainer);
    }

    // ========== 状态时间线区 ==========
    auto* timelineTitle = new QLabel(QStringLiteral("状态时间线"), &dlg);
    timelineTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #0f172a; border-left: 3px solid #b45309; padding-left: 8px;");
    dlgLayout->addWidget(timelineTitle);

    struct TimelineNode {
        QString statusName;
        QDateTime time;
        QString operatorName;
        int nodeStatus;  // 0=已完成, 1=当前, 2=未到达
    };
    QList<TimelineNode> nodes;

    nodes.append({WorkOrderStatus::label(WorkOrderStatus::Pending), createTime, reporterName, 0});

    if (acceptTime.isValid()) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Accepted), acceptTime, QString(), 0});
    } else if (status == WorkOrderStatus::Pending) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Accepted), QDateTime(), QString(), 1});
    } else {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Accepted), QDateTime(), QString(), 2});
    }

    if (assignTime.isValid()) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Assigned), assignTime, assigneeName, 0});
    } else if (status == WorkOrderStatus::Accepted) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Assigned), QDateTime(), QString(), 1});
    } else if (status > WorkOrderStatus::Accepted && status != WorkOrderStatus::Rejected) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Assigned), QDateTime(), QString(), 2});
    }

    if (status >= WorkOrderStatus::Processing && status != WorkOrderStatus::Rejected) {
        if (assignTime.isValid()) {
            nodes.append({WorkOrderStatus::label(WorkOrderStatus::Processing), assignTime, assigneeName, 0});
        }
    } else if (status == WorkOrderStatus::Assigned) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Processing), QDateTime(), assigneeName, 1});
    } else if (status < WorkOrderStatus::Assigned && status != WorkOrderStatus::Rejected) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Processing), QDateTime(), QString(), 2});
    }

    if (finishTime.isValid()) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Completed), finishTime, assigneeName, 0});
    } else if (status == WorkOrderStatus::Processing) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Completed), QDateTime(), QString(), 1});
    } else if (status < WorkOrderStatus::Processing && status != WorkOrderStatus::Rejected) {
        nodes.append({WorkOrderStatus::label(WorkOrderStatus::Completed), QDateTime(), QString(), 2});
    }

    if (status == WorkOrderStatus::Closed || status == WorkOrderStatus::Evaluated) {
        QDateTime closeT = closeTime.isValid() ? closeTime : finishTime;
        nodes.append({WorkOrderStatus::label(status), closeT, assigneeName, 0});
    }

    auto* timelineWidget = new QWidget(&dlg);
    timelineWidget->setStyleSheet("background: #fafafa; border: 1px solid #e2e8f0; border-radius: 6px;");
    auto* timelineLayout = new QVBoxLayout(timelineWidget);
    timelineLayout->setContentsMargins(20, 16, 20, 16);
    timelineLayout->setSpacing(0);

    for (int i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];

        auto* nodeWidget = new QWidget(timelineWidget);
        nodeWidget->setStyleSheet("background: transparent;");
        auto* nodeLayout = new QHBoxLayout(nodeWidget);
        nodeLayout->setContentsMargins(0, 0, 0, 0);
        nodeLayout->setSpacing(12);

        auto* dotContainer = new QWidget(nodeWidget);
        dotContainer->setFixedWidth(20);
        dotContainer->setStyleSheet("background: transparent;");
        QVBoxLayout* dotLayout = new QVBoxLayout(dotContainer);
        dotLayout->setContentsMargins(0, 0, 0, 0);
        dotLayout->setSpacing(0);

        if (i > 0) {
            auto* lineTop = new QWidget(dotContainer);
            lineTop->setFixedWidth(2);
            lineTop->setFixedHeight(8);
            QString lineColor = (nodes[i - 1].nodeStatus == 0) ? "#15803d" : "#e2e8f0";
            lineTop->setStyleSheet(QString("background: %1; border: none;").arg(lineColor));
            dotLayout->addWidget(lineTop, 0, Qt::AlignHCenter);
        }

        auto* dot = new QLabel(dotContainer);
        dot->setFixedSize(12, 12);
        QString dotColor;
        if (node.nodeStatus == 0) {
            dotColor = "#15803d";
        } else if (node.nodeStatus == 1) {
            dotColor = "#b45309";
        } else {
            dotColor = "#e2e8f0";
        }
        dot->setStyleSheet(QString("background: %1; border: 2px solid %2; border-radius: 4px;").arg(dotColor, dotColor));
        dotLayout->addWidget(dot, 0, Qt::AlignHCenter);

        if (i < nodes.size() - 1) {
            auto* lineBot = new QWidget(dotContainer);
            lineBot->setFixedWidth(2);
            lineBot->setFixedHeight(16);
            QString lineColor = (node.nodeStatus == 0) ? "#15803d" : "#e2e8f0";
            lineBot->setStyleSheet(QString("background: %1; border: none;").arg(lineColor));
            dotLayout->addWidget(lineBot, 0, Qt::AlignHCenter);
        }

        nodeLayout->addWidget(dotContainer);

        auto* textWidget = new QWidget(nodeWidget);
        textWidget->setStyleSheet("background: transparent;");
        QVBoxLayout* textLayout = new QVBoxLayout(textWidget);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(2);

        auto* statusLabel = new QLabel(node.statusName, textWidget);
        QString statusLabelColor = (node.nodeStatus == 2) ? "#64748b" : "#0f172a";
        statusLabel->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600; background: transparent;").arg(statusLabelColor));
        textLayout->addWidget(statusLabel);

        QString timeText = node.time.isValid() ? node.time.toString("yyyy-MM-dd hh:mm") : QStringLiteral("待处理");
        QString opText = node.operatorName.isEmpty() ? QString() : QStringLiteral("  ·  ") + node.operatorName;
        auto* timeLabel = new QLabel(timeText + opText, textWidget);
        timeLabel->setStyleSheet("color: #64748b; font-size: 12px; background: transparent;");
        textLayout->addWidget(timeLabel);

        nodeLayout->addWidget(textWidget, 1);
        timelineLayout->addWidget(nodeWidget);
    }

    dlgLayout->addWidget(timelineWidget);

    if (!resultDesc.isEmpty()) {
        auto* resultTitle = new QLabel(QStringLiteral("处理结果"), &dlg);
        resultTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #0f172a; border-left: 3px solid #15803d; padding-left: 8px;");
        dlgLayout->addWidget(resultTitle);

        auto* resultEdit = new QTextEdit(&dlg);
        resultEdit->setReadOnly(true);
        resultEdit->setPlainText(resultDesc);
        resultEdit->setStyleSheet("QTextEdit { background: #f6ffed; border: 1px solid #d9f7be; border-radius: 4px;"
            " padding: 10px; font-size: 13px; color: #14532d; }");
        resultEdit->setFixedHeight(60);
        dlgLayout->addWidget(resultEdit);
    }

    dlgLayout->addStretch();
    auto* btnRow = new QHBoxLayout();
    btnRow->addStretch();

    // 根据当前状态和权限显示操作按钮
    WorkOrderDetailActions::setupActionButtons(dlg, btnRow, workOrderId, status, reporterId, assignTo, title);

    auto* closeBtn = new QPushButton(QStringLiteral("关闭"), &dlg);
    closeBtn->setProperty("cssClass", "primary");
    closeBtn->setCursor(Qt::PointingHandCursor);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    dlgLayout->addLayout(btnRow);

    dlg.exec();
}
