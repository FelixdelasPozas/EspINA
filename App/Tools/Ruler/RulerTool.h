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

#ifndef RULER_TOOL_H_
#define RULER_TOOL_H_

// ESPINA
#include <Support/Widgets/Tool.h>
#include <GUI/View/ViewState.h>

namespace ESPINA
{
  class RenderView;
  class RulerWidget;

  class RulerTool
  : public Tool
  {
    Q_OBJECT
  public:
    /** \brief RulerTool class constructor.
     * \param[in] viewState
     */
    explicit RulerTool(ViewStateSPtr viewState);

    /** \brief RulerTool class destructor.
     *
     */
    virtual ~RulerTool();

    virtual QList<QAction *> actions() const override;

    virtual void abortOperation() override;

  public slots:
    /** \brief Initializes/De-initializes tool.
     * \param[in] value true to initialize tool, false to de-initialize.
     */
    void initTool(bool value);

    /** \brief Updates the ruler widget.
     *
     */
    void selectionChanged();

  private:
    virtual void onToolEnabled ( bool enabled );

  private:
    ViewStateSPtr    m_viewState;
    QAction         *m_action;
    EspinaWidgetSPtr m_widget;
    SelectionSPtr    m_selection;
    EventHandlerSPtr m_handler;
  };

  class RulerEventHandler
  : public EventHandler
  {
  public:
    /** \brief RulerEventHandler class constructor.
     *
     */
    RulerEventHandler()
    {}

    /** \brief RulerEventHadler class destructor.
     *
     */
    ~RulerEventHandler()
    {}


    virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;
  };

  using RulerToolPtr  = RulerTool *;
  using RulerToolSPtr = std::shared_ptr<RulerTool>;

} /* namespace ESPINA */
#endif /* RULERTOOL_H_ */
