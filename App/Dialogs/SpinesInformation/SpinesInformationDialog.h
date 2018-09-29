/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_SPINESINFORMATION_SPINESINFORMATIONDIALOG_H_
#define APP_DIALOGS_SPINESINFORMATION_SPINESINFORMATIONDIALOG_H_

// ESPINA
#include <Extensions/SkeletonInformation/DendriteInformation.h>
#include <Support/Context.h>

// Qt
#include <QDialog>
#include <QTableWidgetItem>

class QCloseEvent;
class QTableWidget;

namespace ESPINA
{
  /** \class SpinesInformationDialog
   * \brief Spines information report dialog.
   *
   */
  class SpinesInformationDialog
  : public QDialog
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief SpinesInformationDialog class constructor.
       * \param[in] input List of dendrites, at least one with spines.
       * \param[in] context Application context.
       *
       */
      explicit SpinesInformationDialog(SegmentationAdapterList input, Support::Context &context);

      /** \brief SpinesInformationDialog class virtual destructor.
       *
       */
      virtual ~SpinesInformationDialog()
      {};

    protected:
      virtual void closeEvent(QCloseEvent *event) override;

    private slots:
      /** \brief Saves table contents to a file on disk in CSV or Excel format.
       *
       */
      void onExportButtonClicked();

      /** \brief Updates the table values associated with a a segmentation when changed.
       *
       */
      void onOutputModified();

      /** \brief Updates the table values associated to the involved segmentations in the connection, if included.
       * \param[in] connection Added connection.
       *
       */
      void onConnectionModified(Connection connection);

      /** \brief Test the added segmentations for inclusion on the table and adds them.
       * \param[in] items List of segmentations added to the model.
       *
       */
      void onSegmentationsAdded(ViewItemAdapterSList items);

      /** \brief Test the removed segmentations for exclusion from the table and removes them.
       * \param[in] items List of segmentations removed from the model.
       *
       */
      void onSegmentationsRemoved(ViewItemAdapterSList items);

      /** \brief Focus the views on the segmentation that contains the spine of the given row.
       * \param[in] row Row of the double-clicked spine.
       *
       */
      void focusOnActor(int row);

    private:
      class Item;

      /** \brief Helper method to connect model signals.
       *
       */
      void connectSignals();

      /** \brief Helper method that creates a GUI similar to ther tabular reports.
       *
       */
      void createGUI();

      /** \brief Saves the contents of the table to a file on disk with the given filename in CSV format.
       * \param[in] filename Name of the file on disk.
       *
       */
      void saveToCSV(const QString &filename);

      /** \brief Saves the contents of the table to a file on disk with the given filename in Excel format.
       * \param[in] filename Name of the file on disk.
       *
       */
      void saveToXLS(const QString &filename);

      /** \brief Helper method that obtains the spines information and fills the table.
       *
       */
      void computeInformation();

      /** \brief Fills the table with information.
       *
       */
      void refreshTable();

      using SpinesList = QList<Extensions::DendriteSkeletonInformation::SpineInformation>;

      SegmentationAdapterList                  m_segmentations; /** input segmentations.    */
      QTableWidget                            *m_table;         /** table widget.           */
      QMap<SegmentationAdapterPtr, SpinesList> m_spinesMap;     /** spines information map. */
  };

  /** \class SpinesInformationDialog::Item
   * \brief Reimplementation of a table widget item, needed for sorting the table.
   *
   */
  class SpinesInformationDialog::Item
  : public QTableWidgetItem
  {
    public:
      /** \brief Item class constructor.
       *
       */
      explicit Item(const QString &info)
      : QTableWidgetItem{info}
      {};

      virtual bool operator<(const QTableWidgetItem &other) const
      {
        if(row() == 0)
        {
          auto data  = this->data(Qt::DisplayRole).toString();
          auto oData = other.data(Qt::DisplayRole).toString();

          if(data.endsWith(" (Truncated)", Qt::CaseInsensitive)) data = data.left(data.length()-12);
          if(oData.endsWith(" (Truncated)", Qt::CaseInsensitive)) oData = oData.left(data.length()-12);

          return (data < oData);
        }

        if(row() == 1)
        {
          auto data  = this->data(Qt::DisplayRole).toString();
          auto oData = other.data(Qt::DisplayRole).toString();

          auto diff = data.length() - oData.length();
          if(diff < 0) return true;
          if(diff > 0) return false;
          return (data < oData);
        }

        return QTableWidgetItem::operator<(other);
      }
  };

} // namespace ESPINA

#endif // APP_DIALOGS_SPINESINFORMATION_SPINESINFORMATIONDIALOG_H_
