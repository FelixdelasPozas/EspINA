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

#ifndef APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCOUNTDIALOG_H_
#define APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCOUNTDIALOG_H_

// ESPINA
#include <Support/Context.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QDialog>
#include <ui_ConnectionCountDialog.h>

namespace ESPINA
{
  /** \class ConnectionCountDialog
   * \brief Display the connection count and classification of connections in the session.
   *
   */
  class ConnectionCountDialog
  : public QDialog
  , private Support::WithContext
  , private Ui::ConnectionCountDialog
  {
      Q_OBJECT
    public:
      /** \brief ConnectionCountDialog class constructor.
       * \param[in] context application context.
       * \param[in] parent raw pointer of the QWidget parent of this one.
       * \param[in] flags dialog flags.
       */
      explicit ConnectionCountDialog(Support::Context &context, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget(), Qt::WindowFlags flags = Qt::WindowFlags());

      /** \brief ConnectionCountDialog class virtual destructor.
       *
       */
      virtual ~ConnectionCountDialog()
      {}

    protected:
      virtual void closeEvent(QCloseEvent *event) override;

    private slots:
      /** \brief Centers the application views to the clicked segmentation.
       * \param[in] item activated item.
       *
       */
      void onItemDoubleClicked(QListWidgetItem *item);

      /** \brief Modifies the lists with the insertion of the newly added segmentations.
       * \param[in] segmentations added segmentations list.
       *
       */
      void onSegmentationsAdded(ViewItemAdapterSList segmentations);

      /** \brief Empties and fills the lists again.
       *
       */
      void updateList();

    private:
      /** \brief Helper method to connect the signals to the slots.
       *
       */
      void connectSignals();

      /** \brief Helper method to add a segmentation to the lists.
       * \param[in] segmentation SegmentationAdapter raw pointer.
       *
       */
      void addSegmentationToLists(const SegmentationAdapterSPtr segmentation);

      /** \brief Returns the row of the segmentation in the list or -1 if not in the list widget.
       * \param[in] list QListWidget object.
       * \param[in] segmentation segmentation adapter object.
       *
       */
      int segmentationIndexInList(QListWidget *list, const SegmentationAdapterPtr segmentation);

      /** \brief Helper method to update the label widgets with the segmentations count.
       *
       */
      void updateLabels();
  };

} // namespace ESPINA

#endif // APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCOUNTDIALOG_H_
