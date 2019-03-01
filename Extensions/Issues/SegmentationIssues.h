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
    : public Core::SegmentationExtension
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
        /** \brief SegmentationIssues class virtual destructor.
         *
         */
        virtual ~SegmentationIssues()
        {}

        virtual Type type() const override
        { return TYPE; }

        virtual bool invalidateOnChange() const override
        { return false; }

        virtual State state() const override
        { return State(); }

        virtual Snapshot snapshot() const override
        { return Snapshot(); }

        virtual const TypeList dependencies() const override
        { return TypeList(); }

        virtual bool validCategory(const QString& classification) const override
        { return true; }

        virtual bool validData(const OutputSPtr output) const
        { return true; }

        virtual const InformationKeyList availableInformation() const override;

        virtual const QString toolTipText() const override;

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
        /** \brief SegmentationIssues class constructor.
         * \param[in] infoCache cache object.
         *
         */
        explicit SegmentationIssues(const InfoCache& infoCache = InfoCache());

        IssueList m_issues; /** list of found issues for the segmentation. */

        friend class SegmentationIssuesFactory;
    };
  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_ISSUES_H
