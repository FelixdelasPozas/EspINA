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

      /** \brief Connects to the newly added segmentation if its a dendrite or axon and updates the list.
       * \param[in] segmentations Segmentations added to the model.
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

      /** \brief Helper method to add synapse segmentations to the lists.
       *
       */
      void addSegmentationsToLists();

      /** \brief Helper method to update the label widgets with the segmentations count.
       *
       */
      void updateLabels();

      /** \brief Set the invalid synapses group visible if there are erroneously connected synapses. If there is none
       * the group will be invisible.
       *
       */
      void updateInvalidVisibility();
  };

} // namespace ESPINA

#endif // APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCOUNTDIALOG_H_
