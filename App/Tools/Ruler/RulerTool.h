/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef RULERTOOL_H_
#define RULERTOOL_H_

// ESPINA
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>

namespace ESPINA
{
  class ViewManager;
  class RenderView;
  class RulerWidget;

  class RulerTool
  : public Tool
  {
    Q_OBJECT
    public:
			/* \brief RulerTool class constructor.
			 * \param[in] viewManager, view manager smart pointer.
			 */
      explicit RulerTool(ViewManagerSPtr viewManager);

      /* \brief RulerTool class destructor.
       *
       */
      virtual ~RulerTool();

      /* \brief Implements Tool::setEnabled().
       *
       */
      virtual void setEnabled(bool value);

      /* \brief Implements Tool::enabled().
       *
       */
      virtual bool enabled() const;

      /* \brief Implements Tool::actions().
       *
       */
      virtual QList<QAction *> actions() const;

    public slots:
			/* \brief Initializes/De-initializes tool.
			 * \param[in] value, true to initialize tool, false to de-initialize.
			 */
      void initTool(bool value);

      /* \brief Updates the ruler widget.
       *
       */
      void selectionChanged();

    private:
      bool             m_enabled;
      QAction         *m_action;
      EspinaWidgetSPtr m_widget;
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

      /* \brief Overrides EventHandler::setInUse().
       *
       */
      virtual void setInUse(bool value) override;

      /* \brief Overrides EventHandler::filterEvent().
       *
       */
      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;
  };

  using RulerToolPtr  = RulerTool *;
  using RulerToolSPtr = std::shared_ptr<RulerTool>;

} /* namespace ESPINA */
#endif /* RULERTOOL_H_ */
