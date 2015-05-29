/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_TOOL_H
#define ESPINA_TOOL_H

#include "Support/EspinaSupport_Export.h"

#include <Support/Context.h>

#include <GUI/Types.h>
#include <GUI/View/EventHandler.h>
#include <GUI/Model/SegmentationAdapter.h>

#include <Core/MultiTasking/TaskGroupProgress.h>

// C++
#include <memory>

// Qt
#include <QCursor>
#include <QWidgetAction>

class QHBoxLayout;
class QAction;
class QEvent;
class QIcon;
class QPushButton;
class QString;

namespace ESPINA
{
  class RenderView;

  namespace Support
  {
    namespace Widgets
    {
      class EspinaSupport_EXPORT ProgressTool
      : public QObject
      , protected WithContext
      {
        Q_OBJECT
      public:
        class NestedWidgets
        : public QWidgetAction
        {
        public:
          explicit NestedWidgets(QObject *parent);

          void addWidget(QWidget *widget);

          bool isEmpty() const;

        private:
          QHBoxLayout *m_layout;
        };

      public:
        explicit ProgressTool(const QIcon &icon, const QString &tooltip, Context &context);

        explicit ProgressTool(const QString &icon, const QString &tooltip, Context &context);

        virtual ~ProgressTool();

        /** \brief Enables/Disables the tool
         * \param[in] value true to enable false otherwise.
         *
         * Disabled tools may still report progess of pending operations
         *
         */
        void setEnabled(bool value);

        /** \brief Returns true if the tool is enabled, false otherwise.
         *
         */
        bool isEnabled() const;

        /** \brief Changes tool behaviour to be checkable or not
         *
         *  Checked tools will display available options
         *
         */
        void setCheckable(bool value);

        void setToolTip(const QString &tooltip);

        QList<QAction *> actions() const;

        virtual void abortOperation() {}

      public slots:
        void setProgress(int value);

        void setChecked(bool value);

      signals:
        void triggered(bool value);

        void toggled(bool value);

      protected:
        void addSettingsWidget(QWidget *widget);

        void showTaskProgress(TaskSPtr task);

        void setEventHandler(EventHandlerSPtr handler);

        void activateEventHandler();

        void deactivateEventHandler();

      private slots:
        void onActionToggled(bool value);

        void onEventHandlerInUse(bool isUsed);

      private:
        GUI::Widgets::ProgressAction *m_action;
        NestedWidgets                *m_settings;

        EventHandlerSPtr m_handler;

        Core::MultiTasking::TaskGroupProgress m_taskProgress;
      };
    }
  }

  using ToolSPtr  = std::shared_ptr<Support::Widgets::ProgressTool>;
  using ToolSList = QList<ToolSPtr>;


} // namespace ESPINA

#endif // ESPINA_TOOL_H
