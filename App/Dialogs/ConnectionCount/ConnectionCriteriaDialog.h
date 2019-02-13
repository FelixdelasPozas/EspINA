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
#include <GUI/Dialogs/DefaultDialogs.h>

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
      explicit ConnectionCriteriaDialog(const ModelAdapterSPtr model, const QStringList &criteria, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget());

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

      /** \brief Returns the hue value of the valid color.
       *
       */
      const short validColor() const;

      /** \brief Returns the hue value of the invalid color.
       *
       */
      const short invalidColor() const;

      /** \brief Returns the hue value of the unconnected color.
       *
       */
      const short unconnectedColor() const;

      /** \brief Returns the hue value of the incomplete color.
       *
       */
      const short incompleteColor() const;

      /** \brief Sets the valid color
       * \param[in] hue Color hue value.
       *
       */
      const void setValidColor(const short hue) const;

      /** \brief Sets the invalid color
       * \param[in] hue Color hue value.
       *
       */
      const void setInvalidColor(const short hue) const;

      /** \brief Sets the unconnected color
       * \param[in] hue Color hue value.
       *
       */
      const void setUnconnectedColor(const short hue) const;

      /** \brief Sets the incomplete color
       * \param[in] hue Color hue value.
       *
       */
      const void setIncompleteColor(const short hue) const;

      /** \brief Shows/hides the color group box.
       * \param[in] enable True to show the group and false to hide it.
       *
       */
      void showColors(const bool enable);

    protected:
      virtual void accept() override;

    private slots:
      void onAddPressed();
      void onRemovePressed();
      void onClearPressed();
      void onHueMoved(int);
      void updateGUI();

    private:
      /** \brief Helper method to create additional dialog widgets.
       * \param[in] model ModelAdapter object.
       *
       */
      void createWidgets(const ModelAdapterSPtr model);

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
