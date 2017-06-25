/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_ADJACENCYMATRIX_ADJACENCYMATRIXTABULARREPORT_H_
#define APP_DIALOGS_ADJACENCYMATRIX_ADJACENCYMATRIXTABULARREPORT_H_

// ESPINA
#include <Support/Widgets/TabularReport.h>
#include <Support/Widgets/TabularReportEntry.h>

// Qt
#include <QItemSelection>

namespace ESPINA
{
  /** \class AdjacencyMatrixTabularReport
   * \brief Tabular report class for adjacency matrix report dialog.
   *
   */
  class AdjacencyMatrixTabularReport
  : public TabularReport
  {
      Q_OBJECT
    protected:
      class Entry;
      class InformationProxy;

    public:
      /** \brief AdjacencyMatrixTabularReport class constructor.
       * \param[in] segmentations list of selected segmentations.
       * \param[in] context application context.
       *
       */
      explicit AdjacencyMatrixTabularReport(SegmentationAdapterList segmentations, Support::Context &context);

      /** \brief AdjacencyMatrixTabularReport class virtual destructor.
       *
       */
      virtual ~AdjacencyMatrixTabularReport()
      {}

    private slots:
      /** \brief Updates the application selection depending on the selected report cells.
       * \param[in] selected group of selected items in the model.
       * \param[in] deselected group of deselected items (previous selected items).
       *
       */
      virtual void updateSelection(QItemSelection selected, QItemSelection deselected);

    protected:
      /** \brief Saves the tabular report information to disk.
       *
       */
      virtual void exportInformation();

    private:
      /** \brief Create and entry to show the adjacency matrix.
       * \param[in] segmentations segmentations list.
       * \param[in] model current session model.
       *
       */
      void createEntry(const SegmentationAdapterList segmentations, const ModelAdapterSPtr model);

    private:
      const SegmentationAdapterList m_segmentations; /** input segmentations. */

  };


  class AdjacencyMatrixTabularReport::Entry
  : public TabularReport::Entry
  {
      Q_OBJECT
    public:
      /** \brief Entry class constructor.
       * \param[in] segmentations list of segmentations
       * \param[in] parent pointer of the QWidget parent of this one.
       *
       */
      explicit Entry(const SegmentationAdapterList segmentations,
                     const ModelAdapterSPtr        model,
                     QWidget                      *parent);

      /** \brief Entry class virtual destructor.
       *
       */
      virtual ~Entry()
      {};

      virtual int rowCount() const override
      { return m_verticalHeaders.size() + 1; }

      virtual int columnCount() const override
      { return m_horizontalHeaders.size() + 1; }

      virtual GUI::InformationSelector::GroupedInfo availableInformation() override
      { return GUI::InformationSelector::GroupedInfo(); };

      virtual void setInformation(GUI::InformationSelector::GroupedInfo extensionInformations, Core::SegmentationExtension::InformationKeyList informationOrder) override
      {};

      /** \brief Returns the list of segmentations in the horizontal headers.
       *
       */
      const SegmentationAdapterList &horizontalHeaders() const
      { return m_horizontalHeaders; }

      /** \brief Returns the list of segmentations in the vertical headers.
       *
       */
      const SegmentationAdapterList &verticalHeaders() const
      { return m_verticalHeaders; }

    private slots:
      virtual void changeDisplayedInformation() override {};
      virtual void refreshAllInformation() override {};
      virtual void saveSelectedInformation() override {};

    private:
      virtual void refreshGUIImplementation() override {};
      virtual void extractInformation() override;

      SegmentationAdapterList m_horizontalHeaders; /** segmentations in the horizontal headers. */
      SegmentationAdapterList m_verticalHeaders;   /** segmentations in the vertical headers.   */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_ADJACENCYMATRIX_ADJACENCYMATRIXTABULARREPORT_H_
