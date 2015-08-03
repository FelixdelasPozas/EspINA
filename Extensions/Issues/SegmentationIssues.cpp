/*
    Copyright (c) 2015, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// ESPINA
#include "SegmentationIssues.h"
#include <GUI/Utils/Conditions.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;

const QString SegmentationIssues::TYPE   = "SegmentationIssues";

const SegmentationExtension::InformationKey  SegmentationIssues::ISSUES(SegmentationIssues::TYPE, "Issues");
const SegmentationExtension::InformationKey  SegmentationIssues::WARNING(SegmentationIssues::TYPE, "Warning");
const SegmentationExtension::InformationKey  SegmentationIssues::CRITICAL(SegmentationIssues::TYPE, "Critical");

//------------------------------------------------------------------------
SegmentationIssues::SegmentationIssues(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
{
}


//------------------------------------------------------------------------
SegmentationIssues::~SegmentationIssues()
{
}

//------------------------------------------------------------------------
SegmentationExtension::InformationKeyList SegmentationIssues::availableInformation() const
{
  InformationKeyList keys;

  keys << WARNING << CRITICAL << ISSUES;

  return keys;
}

//------------------------------------------------------------------------
QString SegmentationIssues::toolTipText() const
{
  const QString WS  = "&nbsp;"; // White space
  const QString TAB = WS+WS+WS;

  QString toolTip;

  for (auto issue : m_issues)
  {
    auto report = QString("<b>%1:</b><br>%2").arg(issue->description())
                                             .arg(issue->suggestion());

    toolTip += condition(severityIcon(issue->severity()), report);
  }

  return toolTip;
}


//------------------------------------------------------------------------
void SegmentationIssues::addIssue(IssueSPtr issue)
{
  m_issues << issue;
}

//------------------------------------------------------------------------
Issue::Severity SegmentationIssues::highestSeverity() const
{
  Issue::Severity highest = Issue::Severity::NONE;

  for (auto issue : m_issues)
  {
    if (issue->severity() > highest)
    {
      highest = issue->severity();
    }
  }

  return highest;
}

//------------------------------------------------------------------------
QString SegmentationIssues::severityIcon(const Issue::Severity severity, bool slim)
{
  QString icon(":/espina/warning%1%2.svg");

  auto critical = severity == Issue::Severity::CRITICAL?"_critical":"";
  auto size     = slim? "_slim":"";

  return icon.arg(critical, size);
}

//------------------------------------------------------------------------
QVariant SegmentationIssues::cacheFail(const InformationKey& key) const
{
  if (key == ISSUES)
  {
    return m_issues.size();
  }
  else if (key == WARNING)
  {
    return std::count_if(m_issues.begin(), m_issues.end(), [](IssueSPtr issue){return issue->severity() == Issue::Severity::WARNING;});
  }
  else if (key == CRITICAL)
  {
    return std::count_if(m_issues.begin(), m_issues.end(), [](IssueSPtr issue){return issue->severity() == Issue::Severity::CRITICAL;});
  }

  return QVariant();
}