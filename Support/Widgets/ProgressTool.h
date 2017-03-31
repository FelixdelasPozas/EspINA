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
          class EspinaSupport_EXPORT NestedWidgets
          : public QWidgetAction
          {
          public:
            /** \brief NestedWidgets class constructor.
             * \param[in] parent pointer of the parent widget of this one.
             *
             */
            explicit NestedWidgets(QObject *parent);

            /** \brief NestedWidgets class virtual destructor.
             *
             */
            virtual ~NestedWidgets();

            /** \brief Adds a given widget to the container.
             * \param[in] widget pointer to the QWidget to add.
             *
             */
            void addWidget(QWidget *widget);

            /** \brief Returns true if there is no widgets in the container and false otherwise.
             *
             */
            bool isEmpty() const;

          private:
            QHBoxLayout *m_layout;
          };

        public:
          /** \brief ProgressTool constructor
           *
           * \param[in] id of the tool
           * \param[in] icon for the tool
           * \param[in] tooltip to be display on mouse hover
           * \param[in] context application context
           *
           */
          explicit ProgressTool(const QString &id, const QIcon &icon, const QString &tooltip, Context &context);

          /** \brief ProgressTool constructor
           *
           * \param[in] id of the tool
           * \param[in] icon for the tool
           * \param[in] tooltip to be display on mouse hover
           * \param[in] context application context
           *
           */
          explicit ProgressTool(const QString &id, const QString &icon, const QString &tooltip, Context &context);

          /** \brief ProgressTool class virtual destructor.
           *
           */
          virtual ~ProgressTool();

          /** \brief Enables/Disables the tool
           * \param[in] value true to enable false otherwise.
           *
           * Disabled tools may still report progress of pending operations
           */
          virtual void setEnabled(bool value);

          /** \brief Returns true if the tool is enabled, false otherwise.
           *
           */
          bool isEnabled() const;

          /** \brief Changes tool behavior to be checkable or not
           *
           *  Checked tools will display available options
           */
          void setCheckable(bool value);

          bool isChecked() const;

          /** \brief Changes tool behavior to be explicit or not
           *
           *  There can only be one exclusive tool toggled in EspINA
           */
          void setExclusive(bool value);

          /** \brief Sets the grouping criteria.
           * \param[in] name name of the tool.
           * \param[in] group name of the group.
           *
           */
          void setOrder(const QString &name, const QString &group = QString());

          /** \brief Returns the name of the group this tool belongs to.
           *
           */
          QString groupWith() const;

          /** \brief Returns the name position of the tool.
           *
           */
          QString positionName() const;

          /** \brief Sets the tooltip text of the tool.
           * \param[in] tooltip tooltip text.
           *
           */
          void setToolTip(const QString &tooltip);

          /** \brief Sets the icon of the tool.
           * \param[in] icon tool icon.
           *
           */
          void setIcon(const QIcon &icon);

          /** \brief Returns the icon of the tool.
           *
           */
          QIcon icon() const;

          /** \brief Returns the list of settings actions of the tool.
           *
           */
          QList<QAction *> actions() const;

          /** \brief Helper method to execute when the toolgroup this tool belongs to is activated.
           *
           */
          virtual void onToolGroupActivated()
          {};

          /** \brief Helper method to execute when the user has cancelled the current action.
           *
           *  Reimplement this method if your tool enters a state that can be aborted.
           */
          virtual void abortOperation()
          {};

          /** \brief Adds the key sequence to the list used for the activation of this tool.
           * \param[in] keySequence QKeySequence object.
           *
           */
          void setShortcut(QKeySequence keySequence);

          /** \brief Returns list of the key sequences used for the activation of this tool.
           *
           */
          QList<QKeySequence> shortcuts() const;

          /** \brief Helper method to execute when a tool is executed with exclusivity.
           * \param[in] tool tool being activated.
           *
           */
          void onExclusiveToolInUse(ProgressTool *tool);

          /** \brief Restores the settings of the tool from the given QSettings object.
           * \param[in] settings.
           *
           * Reimplement this method on your tool to restore button and nested widgets state settings.
           * Use the method ProgressTool::restoreCheckedState(std::shared_ptr<QSettings> settings) only for
           * settings and not for tools with widgets.
           * If your tool only has a state button use the base class GenericTooglableTool.
           * If your tool is a representation swith use the base class RepresentationSwitch.
           *
           */
          virtual void restoreSettings(std::shared_ptr<QSettings> settings)
          {}

          /** \brief Saves the settings of the tool to the given QSettings object.
           * \param[in] settings.
           *
           * Reimplement this method on your tool to restore button and nested widgets state settings.
           * Use the method ProgressTool::saveCheckedState(std::shared_ptr<QSettings> settings) only for
           * settings and not for tools with widgets.
           * If your tool only has a state button use the base class GenericTooglableTool.
           * If your tool is a representation swith use the base class RepresentationSwitch.
           *
           */
          virtual void saveSettings(std::shared_ptr<QSettings> settings)
          {}

          /** \brief Returns the unique identifier of the tool.
           *
           */
          const QString &id() const;

        public slots:
          /** \brief Helper method to launch the action of the tool.
           *
           */
          void trigger();

          /** \brief Sets the progress value of the tool.
           * \param[in] value progress value in [0,100].
           *
           */
          void setProgress(int value);

          /** \brief Checks/Unchecks the tool.
           * \param[in] value true to check and false otherwise.
           *
           */
          void setChecked(bool value);

        signals:
          void triggered(bool value);

          void toggled(bool value);

          void exclusiveToolInUse(Support::Widgets::ProgressTool *tool);

        protected slots:
          /** \brief Updates the tool task progress.
           *
           */
          void showTaskProgress(TaskSPtr task);

        protected:
          /** \brief Adds a widget to the group of settings widgets of this tool.
           * \param[in] widget widget object pointer.
           *
           */
          void addSettingsWidget(QWidget *widget);

          /** \brief Sets the event handler to be activated with the tool.
           * \param[in] handler event handler object.
           *
           */
          void setEventHandler(EventHandlerSPtr handler);

          /** \brief Activates the event handler of the tool.
           *
           */
          void activateEventHandler();

          /** \brief Deactivates the event handler of the tool.
           *
           */
          void deactivateEventHandler();

          /** \brief Helper method to save tool's checked state
           * \param[inout] settings QSettings object.
           *
           */
          void saveCheckedState(std::shared_ptr<QSettings> settings);

          /** \brief Helper method to restore tool's checked state
           * \param[inout] settings QSettings object.
           *
           */
          void restoreCheckedState(std::shared_ptr<QSettings> settings);

        private slots:
          /** \brief Helper method to show/hide the settings widgets and enable/disable the event handler
           * when the tool is activated/deactivated.
           * \param[in] value tool activation state.
           *
           */
          void onActionToggled(bool value);

          /** \brief Helper to update the state of the tool when the event handler changes state.
           * \param[in] isUsed event handler activation state.
           *
           */
          void onEventHandlerInUse(bool isUsed);

        private:
          GUI::Widgets::ProgressAction *m_action;               /** tool action.                                       */
          NestedWidgets                *m_settings;             /** tool settings widgets.                             */
          bool                          m_isExlusive;           /** true if the tool is exclusive and false otherwise. */
          QString                       m_positionName;         /** tool position id in the toolgroup.                 */
          QString                       m_groupName;            /** name of the toolgroup this tool belongs to.        */
          const QString                 m_id;                   /** id of the tool.                                    */
          QList<QKeySequence>           m_shortcutSequences;    /** keyboard shortcuts of the tool .                   */

          EventHandlerSPtr              m_handler;              /** event handler of the tool.                         */

          Core::MultiTasking::TaskGroupProgress m_taskProgress; /** tool progress manager.                             */
      };
    } // namespace Widgets
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_PROGRESS_TOOL_H
