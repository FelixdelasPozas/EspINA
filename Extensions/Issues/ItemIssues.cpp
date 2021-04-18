/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include <Extensions/Issues/ItemIssues.h>
#include <GUI/Utils/Format.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Utils::Format;

const QString SegmentationIssues::TYPE   = "SegmentationIssues";

const SegmentationExtension::InformationKey  SegmentationIssues::ISSUES(SegmentationIssues::TYPE, "Issues");
const SegmentationExtension::InformationKey  SegmentationIssues::WARNING(SegmentationIssues::TYPE, "Warning");
const SegmentationExtension::InformationKey  SegmentationIssues::CRITICAL(SegmentationIssues::TYPE, "Critical");

const QString StackIssues::TYPE   = "StackIssues";

const StackExtension::InformationKey  StackIssues::ISSUES(StackIssues::TYPE, "Issues");
const StackExtension::InformationKey  StackIssues::WARNING(StackIssues::TYPE, "Warning");
const StackExtension::InformationKey  StackIssues::CRITICAL(StackIssues::TYPE, "Critical");

//------------------------------------------------------------------------
SegmentationIssues::SegmentationIssues(const InfoCache& infoCache)
: SegmentationExtension{infoCache}
{
}

//------------------------------------------------------------------------
const SegmentationExtension::InformationKeyList SegmentationIssues::availableInformation() const
{
  InformationKeyList keys;

  keys << SegmentationIssues::WARNING << SegmentationIssues::CRITICAL << SegmentationIssues::ISSUES;

  return keys;
}

//------------------------------------------------------------------------
const QString SegmentationIssues::toolTipText() const
{
  QString toolTip;

  auto operation = [&toolTip](const IssueSPtr issue)
  {
    auto report = QString("<b>%1</b><br>(%2)").arg(issue->description())
                                              .arg(issue->suggestion());

    toolTip += createTable(severityIcon(issue->severity()), report);
  };
  std::for_each(m_issues.constBegin(), m_issues.constEnd(), operation);

  return toolTip;
}

//------------------------------------------------------------------------
void SegmentationIssues::addIssue(IssueSPtr issue)
{
  if(issue && issue.get()) m_issues << issue;
}

//------------------------------------------------------------------------
Issue::Severity SegmentationIssues::highestSeverity() const
{
  auto highest = Issue::Severity::NONE;

  auto operation = [&highest](const IssueSPtr issue) { if(issue->severity() > highest) highest = issue->severity(); };
  std::for_each(m_issues.constBegin(), m_issues.constEnd(), operation);

  return highest;
}

//------------------------------------------------------------------------
QString SegmentationIssues::severityIcon(const Issue::Severity severity, bool slim)
{
  QString icon(":/espina/warning%1%2.svg");

  auto critical = severity == Issue::Severity::CRITICAL ? "_critical" : "";
  auto size     = slim ? "_slim" : "";

  return icon.arg(critical, size);
}

//------------------------------------------------------------------------
QVariant SegmentationIssues::cacheFail(const InformationKey& key) const
{
  if (key == SegmentationIssues::ISSUES)
  {
    return m_issues.size();
  }
  else if (key == SegmentationIssues::WARNING)
  {
    return std::count_if(m_issues.constBegin(), m_issues.constEnd(), [](IssueSPtr issue){return issue->severity() == Issue::Severity::WARNING;});
  }
  else if (key == SegmentationIssues::CRITICAL)
  {
    return std::count_if(m_issues.constBegin(), m_issues.constEnd(), [](IssueSPtr issue){return issue->severity() == Issue::Severity::CRITICAL;});
  }

  return QVariant();
}

//------------------------------------------------------------------------
StackIssues::StackIssues(const InfoCache& infoCache)
: StackExtension{infoCache}
{
}

//------------------------------------------------------------------------
const StackExtension::InformationKeyList StackIssues::availableInformation() const
{
  InformationKeyList keys;

  keys << StackIssues::WARNING << StackIssues::CRITICAL << StackIssues::ISSUES;

  return keys;
}

//------------------------------------------------------------------------
const QString StackIssues::toolTipText() const
{
  QString toolTip;

  auto operation = [&toolTip](const IssueSPtr issue)
  {
    auto report = QString("<b>%1</b><br>(%2)").arg(issue->description())
                                              .arg(issue->suggestion());

    toolTip += createTable(severityIcon(issue->severity()), report);
  };
  std::for_each(m_issues.constBegin(), m_issues.constEnd(), operation);

  return toolTip;
}

//------------------------------------------------------------------------
void StackIssues::addIssue(IssueSPtr issue)
{
  if(issue && issue.get()) m_issues << issue;
}

//------------------------------------------------------------------------
Issue::Severity StackIssues::highestSeverity() const
{
  auto highest = Issue::Severity::NONE;

  auto operation = [&highest](const IssueSPtr issue) { if(issue->severity() > highest) highest = issue->severity(); };
  std::for_each(m_issues.constBegin(), m_issues.constEnd(), operation);

  return highest;
}

//------------------------------------------------------------------------
QString StackIssues::severityIcon(const Issue::Severity severity, bool slim)
{
  QString icon(":/espina/warning%1%2.svg");

  auto critical = severity == Issue::Severity::CRITICAL ? "_critical" : "";
  auto size     = slim ? "_slim" : "";

  return icon.arg(critical, size);
}

//------------------------------------------------------------------------
QVariant StackIssues::cacheFail(const InformationKey& key) const
{
  if (key == StackIssues::ISSUES)
  {
    return m_issues.size();
  }
  else if (key == StackIssues::WARNING)
  {
    return std::count_if(m_issues.constBegin(), m_issues.constEnd(), [](IssueSPtr issue){return issue->severity() == Issue::Severity::WARNING;});
  }
  else if (key == StackIssues::CRITICAL)
  {
    return std::count_if(m_issues.constBegin(), m_issues.constEnd(), [](IssueSPtr issue){return issue->severity() == Issue::Severity::CRITICAL;});
  }

  return QVariant();
}
