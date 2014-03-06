/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

#ifndef RULERTOOL_H_
#define RULERTOOL_H_

// EspINA
#include <Support/Tool.h>
#include <Support/ViewManager.h>

namespace EspINA
{
  class ViewManager;
  class RenderView;
  class RulerWidget;
  
  class RulerTool
  : public Tool
  {
    Q_OBJECT
    public:
      explicit RulerTool(ViewManagerSPtr);
      virtual ~RulerTool();

      /* \brief Implements Tool::setEnabled.
       *
       */
      virtual void setEnabled(bool value);

      /* \brief Implements Tool::enabled.
       *
       */
      virtual bool enabled() const;

      /* \brief Implements Tool::actions.
       *
       */
      virtual QList<QAction *> actions() const;

    public slots:
      void initTool(bool value);
      void selectionChanged();
      void selectedElementChanged();

    private:
      bool             m_enabled;
      QAction         *m_action;
      RulerWidget     *m_widget;
      ViewManagerSPtr  m_viewManager;
      SelectionSPtr    m_selection;
      EventHandlerSPtr m_handler;
  };

  class RulerEventHandler
  : public EventHandler
  {
    public:
      /* \brief RulerEventHandler class constructor.
       *
       */
      RulerEventHandler()
      {}

      /* \brief RulerEventHadler class destructor.
       *
       */
      ~RulerEventHandler()
      {}

      /* \brief Implements EventHandler::setInUse.
       *
       */
      virtual void setInUse(bool value);

      /* \brief Implements EventHandler::filterEvent.
       *
       */
      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);
  };

  using RulerToolPtr  = RulerTool *;
  using RulerToolSPtr = std::shared_ptr<RulerTool>;

} /* namespace EspINA */
#endif /* RULERTOOL_H_ */
