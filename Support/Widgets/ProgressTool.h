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

#ifndef ESPINA_PROGRESS_TOOL_H
#define ESPINA_PROGRESS_TOOL_H

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
        explicit ProgressTool(const QString &id, const QIcon &icon, const QString &tooltip, Context &context);

        explicit ProgressTool(const QString &id, const QString &icon, const QString &tooltip, Context &context);

        virtual ~ProgressTool();

        /** \brief Enables/Disables the tool
         * \param[in] value true to enable false otherwise.
         *
         * Disabled tools may still report progess of pending operations
         */
        void setEnabled(bool value);

        /** \brief Returns true if the tool is enabled, false otherwise.
         *
         */
        bool isEnabled() const;

        /** \brief Changes tool behaviour to be checkable or not
         *
         *  Checked tools will display available options
         */
        void setCheckable(bool value);

        bool isChecked() const;

        /** \brief Changes tool behaviour to be explicit or not
         *
         *  There can only be one excluive tool toogled in EspINA
         */
        void setExclusive(bool value);

        /** \brief Sets the group name which tools are grouped with
         *
         */
        void setGroupWith(const QString &name);

        QString groupWith() const;

        void setToolTip(const QString &tooltip);

        QList<QAction *> actions() const;

        virtual void onToolGroupActivated() {}

        virtual void abortOperation() {}

        /** \brief Sets the key sequence used for the activation of this tool.
         * \param[in] keySequence QKeySequence object.
         */
        void setShortcut(QKeySequence keySequence);

        /** \brief Returns the key sequence used for the activation of this tool.
         *
         */
        QKeySequence shortcut() const;

        void onExclusiveToolInUse(ProgressTool *tool);

        /** \brief Restores the settings of the tool from the given QSettings object.
         * \param[in] settings.
         *
         */
        virtual void restoreSettings(std::shared_ptr<QSettings> settings);

        /** \brief Saves the settings of the tool to the given QSettings object.
         * \param[in] settings.
         *
         */
        virtual void saveSettings(std::shared_ptr<QSettings> settings);

        /** \brief Returns the unique identifier of the tool.
         *
         */
        const QString id() const;

      public slots:
        void trigger();

        void setProgress(int value);

        void setChecked(bool value);

      signals:
        void triggered(bool value);

        void toggled(bool value);

        void exclusiveToolInUse(Support::Widgets::ProgressTool *tool);

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

        bool    m_isExlusive;
        QString m_groupName;
        const QString m_id;
        QKeySequence m_shortcutSequence;

        EventHandlerSPtr m_handler;

        Core::MultiTasking::TaskGroupProgress m_taskProgress;
      };
    }
  }

  using ToolSPtr  = std::shared_ptr<Support::Widgets::ProgressTool>;
  using ToolSList = QList<ToolSPtr>;


} // namespace ESPINA

#endif // ESPINA_PROGRESS_TOOL_H
