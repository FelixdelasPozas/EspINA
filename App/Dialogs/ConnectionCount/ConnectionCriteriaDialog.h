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

#ifndef APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCRITERIADIALOG_H_
#define APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCRITERIADIALOG_H_

// ESPINA
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QDialog>
#include <ui_ConnectionCriteriaDialog.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class CategorySelector;
    }
  }

  class ConnectionCriteriaDialog
  : public QDialog
  , private Ui::ConnectionCriteriaDialog
  {
      Q_OBJECT
    public:
      /** \brief ConnectionCriteriaDialog class constructor.
       * \param[in] model Model adapter.
       * \param[in] criteria Connection criteria.
       * \param[in] parent Raw pointer of the widget parent of this one.
       *
       */
      explicit ConnectionCriteriaDialog(const ModelAdapterSPtr model, const QStringList &criteria, QWidget *parent = nullptr);

      /** \brief ConnectionCriteriaDialog class virtual destructor.
       *
       */
      virtual ~ConnectionCriteriaDialog()
      {};

      /** \brief Returns the selected criteria.
       *
       */
      const QStringList &criteria() const
      { return m_criteria; }

      /** \brief Helper method to return a rich text string of the criteria.
       * \param[in] criteria Connection criteria.
       * \param[in] classification Model's classification adapter.
       *
       */
      static const QString criteriaToText(const QStringList &criteria, const ClassificationAdapterSPtr classification);

    protected:
      virtual void accept() override;

    private slots:
      void onAddPressed();
      void onRemovePressed();
      void onClearPressed();
      void updateGUI();

    private:
      /** \brief Returns true if the current criteria is ambiguous.
       *
       */
      bool isAmbiguous() const;

      /** \brief Helper method to connect UI signals to slots.
       *
       */
      void connectSignals();

      /** \brief Updates the UI and informs if the criteria is ambiguous.
       *
       */
      void updateCriteria();

    private:
      QStringList                     m_criteria;       /** connection selection criteria. */
      GUI::Widgets::CategorySelector *m_selector;       /** category selector.             */
      ClassificationAdapterSPtr       m_classification; /** model's classification.        */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_CONNECTIONCOUNT_CONNECTIONCRITERIADIALOG_H_
