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

#ifndef ESPINA_GUI_WIDGETS_PROGRESS_ACTION_H
#define ESPINA_GUI_WIDGETS_PROGRESS_ACTION_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QWidgetAction>

class QPushButton;

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      /** \class ProgressAction
       * \brief QWidgetAction that reports a progress in it's icon with a small progress bar.
       *
       */
      class EspinaGUI_EXPORT ProgressAction
      : public QWidgetAction
      {
        Q_OBJECT
      public:
        /** \brief ProgressAction class constructor.
         * \param[in] icon action icon (needs to be present in the application's resources file).
         * \param[in] tooltip action tooltip text.
         * \param[in] parent pointer to the object parent of this one.
         *
         */
        explicit ProgressAction(const QString &icon, const QString &tooltip, QObject* parent = nullptr);

        /** \brief ProgressAction class constructor.
         * \param[in] icon action icon.
         * \param[in] tooltip action tooltip text.
         * \param[in] parent pointer to the object parent of this one.
         *
         */
        explicit ProgressAction(const QIcon &icon, const QString &tooltip, QObject* parent = nullptr);

        virtual QWidget* createWidget(QWidget* parent);

        /** \brief Sets the action icon.
         * \param[in] icon action's new icon.
         *
         */
        void setActionIcon(const QIcon &icon);

        /** \brief Sets the action tooltip text.
         * \param[in] tooltip action's new tooltip text.
         *
         */
        void setActionToolTip(const QString &tooltip);

      public slots:
        /** \brief Sets the progress of the action.
         * \param[in] progress progress value in [0-100].
         *
         */
        void setProgress(int progress);

        /** \brief Enables/Disables the action.
         * \param[in] enabled true to enable the action and false otherwise.
         *
         */
        void setActionEnabled(bool enabled);

        /** \brief Checks/Unchecks the action.
         * \param[in] checked true to check the action and false otherwise.
         *
         */
        void setActionChecked(bool checked);

      signals:
        void progressChanged(int value);

        void progressVisibilityChanged(bool visible);

        void actionEnabled(bool enabled);

        void actionChecked(bool checked);

        void iconChanged(const QIcon &icon);

        void toolChanged(const QString &tooltip);

      private:
        /** \brief Creates a tool button with the current action configuration.
         * \param[in] parent pointer to the object parent of this one.
         *
         */
        QPushButton *createActionButton(QWidget *parent = nullptr);

        /** \brief Creates a progress bar to show progress with the current action configuration.
         * \param[in] parent pointer to the object parent of this one.
         *
         */
        void createProgress(QWidget *parent = nullptr);

        /** \brief Returns progress bar height.
         *
         */
        static constexpr int progressHeight();

        /** \brief Returns progress bar margin.
         *
         */
        static constexpr int progressMargin();

        /** \brief Returns progress bar width.
         *
         */
        static constexpr int progressWitdh();

        /** \brief Returns the vertical position inside the button of the progress bar.
         *
         */
        static constexpr int progressVerticalPosition();

        /** \brief Returns true if the progress is in (0,100) and false otherwise.
         * \param[in] progress progress value in [0,100].
         *
         */
        inline bool displayProgress(int progress);

      private:
        int  m_progress;
      };
    }
  }
}

#endif // ESPINA_GUI_WIDGETS_PROGRESS_ACTION_H
