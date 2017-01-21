/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SAS_TABULAR_REPORT_H_
#define SAS_TABULAR_REPORT_H_

#include "AppositionSurfacePlugin_Export.h"

// Plugin
#include <Core/Extensions/AppositionSurfaceExtension.h>

// ESPINA
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/ModelFactory.h>
#include <Support/Widgets/TabularReport.h>
#include <Support/Widgets/TabularReportEntry.h>

namespace ESPINA
{
  /** \class SASTabularReport
   * \brief Implements a tabular report specially for crossing the data of a segmentation and it's SAS.
   *
   */
  class AppositionSurfacePlugin_EXPORT SASTabularReport
  : public TabularReport
  {
    protected:
      class Entry;

    public:
      /** \brief SASTabularReport class constructor.
       *
       */
      SASTabularReport(Support::Context &context,
                       QWidget          *parent = GUI::DefaultDialogs::defaultParentWidget(),
                       Qt::WindowFlags   flags = Qt::WindowFlags{Qt::WindowNoState})
      : TabularReport(context, parent, flags)
      , m_sasTags{context.factory()->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformation()}
      {
        setModel(context.model());
      };

    protected slots:
      virtual void exportInformation() override;

    private:
      virtual void createCategoryEntry(const QString &category);

      static QString extraPath(const QString &file = QString())
      {
        return "Extra/SASInformation/" + file;
      }

    private:
      Core::SegmentationExtension::InformationKeyList m_sasTags;
  };

  class AppositionSurfacePlugin_EXPORT SASTabularReport::Entry
  : public TabularReport::Entry
  {
    public:
      /** \brief Entry class constructor.
       *
       */
      explicit Entry(const QString   &category,
                     ModelAdapterSPtr model,
                     ModelFactorySPtr factory,
                     QWidget         *parent)
      : TabularReport::Entry{category, model, factory, parent}
      {};

      virtual GUI::InformationSelector::GroupedInfo availableInformation() override;

      virtual void setInformation(GUI::InformationSelector::GroupedInfo extensionInformations, Core::SegmentationExtension::InformationKeyList informationOrder) override;

    protected slots:
      virtual void extractInformation() override;

    private:
      Core::SegmentationExtension::KeyList keyValues(const Core::SegmentationExtension::InformationKeyList &keys) const;

      bool isSASExtensions(const Core::SegmentationExtension::Type &type) const;
  };
} // namespace ESPINA

#endif // SAS_TABULAR_REPORT_H_
