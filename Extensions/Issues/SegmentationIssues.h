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

#ifndef ESPINA_SEGMENTATION_ISSUES_H
#define ESPINA_SEGMENTATION_ISSUES_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>
#include "Issues.h"

namespace ESPINA
{
  namespace Extensions
  {
    class EspinaExtensions_EXPORT SegmentationIssues
    : public SegmentationExtension
    {
    public:
      static const Type TYPE;

      // Stores the number of issues of a segmentation
      static const InformationKey ISSUES;

      // Stores the number of warnings
      static const InformationKey WARNING;

      // Stores the number of critical warnings
      static const InformationKey CRITICAL;

    public:
      /** \brief SegmentationIssues class constructor.
       * \param[in] infoCache cache object.
       *
       */
      explicit SegmentationIssues(const InfoCache& infoCache = InfoCache());

      /** \brief SegmentationIssues class virtual destructor.
       *
       */
      virtual ~SegmentationIssues();

      virtual Type type() const override
      { return TYPE; }

      virtual bool invalidateOnChange() const override
      { return false; }

      // NOTE: save to seg? requires further issues management in checker.
      virtual State state() const override
      { return State(); }

      virtual Snapshot snapshot() const override
      { return Snapshot(); }

      virtual TypeList dependencies() const override
      { return TypeList(); }

      virtual bool validCategory(const QString& classification) const override
      { return true; }

      virtual InformationKeyList availableInformation() const override;

      virtual QString toolTipText() const override;

      /** \brief Adds an issue to the list of issues.
       * \param[in] issue issue to add.
       *
       */
      void addIssue(IssueSPtr issue);

      /** \brief Returns the list of issues.
       *
       */
      IssueList issues() const
      { return m_issues; }

      /** \brief Returns the highest severity level of the list of issues.
       *
       */
      Issue::Severity highestSeverity() const;

      /** \brief Returns the icon to the given issue level.
       * \param[in] severity issue severity level.
       * \param[in] slim true to return the slim icon and false to return the normal one.
       *
       */
      static QString severityIcon(const Issue::Severity severity, bool slim = false);

    protected:
      virtual void onExtendedItemSet(SegmentationPtr item) override
      {}

      virtual QVariant cacheFail(const InformationKey& key) const override;

    private:
      IssueList m_issues;
    };
  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_ISSUES_H
