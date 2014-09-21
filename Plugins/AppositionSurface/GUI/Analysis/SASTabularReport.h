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
      /** brief SASTabularReport class constructor.
       *
       */
      SASTabularReport(ModelAdapterSPtr model,
                       ModelFactorySPtr factory,
                       ViewManagerSPtr  viewManager,
                       QWidget         *parent = nullptr,
                       Qt::WindowFlags  flags = Qt::WindowFlags{Qt::WindowNoState})
      : TabularReport(factory, viewManager, parent, flags)
      , m_model{model}
      , m_sasTags{factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformations()}
      {};

      protected slots:
      /** brief Implements TabularReport::exportInformation()
       *
       */
      void exportInformation();

    private:
      /** brief Implements TabularReport::createCategoryEntry(const QString);
       * \param[in] category, QString with category to be created.
       *
       */
      void createCategoryEntry(const QString &category);

      /** brief Implements TabularReport::extraPath(const QString);
       *
       */
      static QString extraPath(const QString &file = QString())
      { return "Extra/SASInformation/" + file; }

      ModelAdapterSPtr m_model;
      SegmentationExtension::InfoTagList m_sasTags;
    };

  class SASTabularReport::Entry
  : public TabularReport::Entry
  {
    public:
      /** brief Entry class constructor.
       *
       */
      explicit Entry(const QString   &category,
                     ModelAdapterSPtr model,
                     ModelFactorySPtr factory)
      : TabularReport::Entry{category, model, factory}
      {};

      /** brief Implements TabularReport::Entry::avalableInformation().
       *
       */
      InformationSelector::GroupedInfo availableInformation();

      /** brief Implements TabularReport::Entry::setInformation().
       *
       */
      void setInformation(InformationSelector::GroupedInfo extensionInformations, QStringList informationOrder);

    private slots:
      /** brief Implements TabularReport::Entry::extractInformation().
       *
       */
      void extractInformation();
  };

} // namespace ESPINA

#endif // SAS_TABULAR_REPORT_H_
