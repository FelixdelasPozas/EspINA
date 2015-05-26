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
#include <GUI/Types.h>
#include <GUI/View/EventHandler.h>

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

  class EspinaSupport_EXPORT Tool
  : public QObject
  {
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
    explicit Tool();

    /** \brief Enables/Disables the tool.
     * \param[in] value true to enable false otherwise.
     *
     */
    void setEnabled2(bool value);

    /** \brief Returns true if the tool is enabled, false otherwise.
     *
     */
    bool isEnabled() const;

    virtual QList<QAction *> actions() const = 0;

    static QAction *createAction( const QString &icon, const QString &tooltip, QObject *parent );

    static QAction *createAction(const QIcon &icon, const QString &tooltip, QObject *parent);

    static QPushButton *createButton(const QString &icon, const QString &tooltip);

    static QPushButton *createButton(const QIcon &icon, const QString &tooltip);

    virtual void abortOperation() = 0;

  private:
    virtual void onToolEnabled(bool enabled) = 0;

  private:
    bool m_enabled;
  };

  namespace Support
  {
    class Context;

    namespace Widgets
    {

      class EspinaSupport_EXPORT ProgressTool
      : public Tool
      {
        Q_OBJECT

      public:
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

        QList<QAction *> actions() const;

      public slots:
        void setProgress(int value);

      signals:
        void triggered(bool value);

        void toggled(bool value);

      protected:
        Context &context();

        void addSettingsWidget(QWidget *widget);

        void showTaskProgress(TaskSPtr task);

        void setEventHandler(EventHandlerSPtr handler);

        void activateEventHandler();

        void deactivateEventHandler();

      private slots:
        void onActionToggled(bool value);

        void onEventHandlerInUse(bool isUsed);

      private:
        Context &m_context;

        GUI::Widgets::ProgressAction *m_action;
        Tool::NestedWidgets          *m_settings;

        EventHandlerSPtr m_handler;

        Core::MultiTasking::TaskGroupProgress m_taskProgress;
      };
    }
  }

  using ToolSPtr  = std::shared_ptr<Tool>;
  using ToolSList = QList<ToolSPtr>;


} // namespace ESPINA

#endif // ESPINA_TOOL_H
