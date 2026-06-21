#include "pages/PageFactory.h"
#include "PagesCommon.h"
#include "services/AuthService.h"

// ========== Governance Pages ==========
BasePage *PageFactory::createGovernancePage(const QString &sub)
{
  if (sub == QStringLiteral("event"))
    return PageFactory::createEventPage();
  if (sub == QStringLiteral("inspection"))
    return PageFactory::buildGovernanceInspection();
  if (sub == QStringLiteral("care"))
    return PageFactory::buildGovernanceCare();
  if (sub == QStringLiteral("supervision"))
    return PageFactory::buildGovernanceSupervision();
  if (sub == QStringLiteral("opinion"))
    return PageFactory::buildGovernanceOpinion();
  if (sub == QStringLiteral("assessment"))
    return PageFactory::buildGovernanceAssessment();

  auto *page = new BasePage();
  auto *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->addWidget(UiKit::createEmptyHintLabel(QStringLiteral("暂无数据"), page));
  return page;
}
