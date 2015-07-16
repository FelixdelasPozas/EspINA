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

// Plugin
#include <Core/Extensions/AppositionSurfaceExtension.h>

// ESPINA
#include <GUI/ModelFactory.h>
#include <Support/Widgets/TabularReport.h>
#include <Support/Widgets/TabularReportEntry.h>

// Qt
#include <QDebug>

namespace ESPINA
{
  class SASTabularReport
  : public TabularReport
  {
    protected:
      class Entry;

    public:
      /** \brief SASTabularReport class constructor.
       *
       */
      SASTabularReport(Support::Context &context,
                       QWidget                *parent = nullptr,
                       Qt::WindowFlags         flags  = Qt::WindowFlags{Qt::WindowNoState})
      : TabularReport(context, parent, flags)
      , m_sasTags{context.factory()->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformation()}
      {
        setModel(context.model());
      };

    protected slots:
      void exportInformation();

    private:
      void createCategoryEntry(const QString &category);

      static QString extraPath(const QString &file = QString())
      { return "Extra/SASInformation/" + file; }

      SegmentationExtension::KeyList m_sasTags;
    };

  class SASTabularReport::Entry
  : public TabularReport::Entry
  {
    public:
      /** \brief Entry class constructor.
       *
       */
      explicit Entry(const QString   &category,
                     ModelAdapterSPtr model,
                     ModelFactorySPtr factory)
      : TabularReport::Entry{category, model, factory}
      {};

      GUI::InformationSelector::GroupedInfo availableInformation();

      void setInformation(GUI::InformationSelector::GroupedInfo extensionInformations, SegmentationExtension::InformationKeyList informationOrder);

    private slots:
      void extractInformation();
  };

} // namespace ESPINA

#endif // SAS_TABULAR_REPORT_H_
