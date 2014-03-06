/*
 * MeasureTool.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ESPINA_MEASURE_TOOL_H_
#define ESPINA_MEASURE_TOOL_H_

// EspINA
#include <GUI/View/Widgets/Measures/MeasureWidget.h>
#include <Support/Tool.h>
#include <Support/ViewManager.h>

class QAction;

namespace EspINA
{

  class MeasureTool
  : public Tool
  {
    Q_OBJECT
  public:
    explicit MeasureTool(ViewManagerSPtr);
    virtual ~MeasureTool();

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
    void initTool(bool);

  signals:
    void stopMeasuring();

  private:
    bool             m_enabled;
    MeasureWidget   *m_widget;
    ViewManagerSPtr  m_viewManager;
    QAction         *m_action;
    EventHandlerSPtr m_handler;
  };

  class MeasureEventHandler
  : public EventHandler
  {
    public:
      explicit MeasureEventHandler(MeasureWidget *widget)
      : m_widget(widget)
      {}

      virtual ~MeasureEventHandler()
      {}

      /* \brief Implements EventHandler::setInUse.
       *
       */
      virtual void setInUse(bool value);

      /* \brief Implements EventHandler::filterEvent.
       *
       */
      virtual bool filterEvent(QEvent *e, RenderView *view=nullptr);

    private:
      MeasureWidget *m_widget;
  };

  using MeasureToolPtr  = MeasureTool *;
  using MeasureToolSPtr = std::shared_ptr<MeasureTool>;

} // namespace EspINA

#endif /* MEASURETOOL_H_ */
