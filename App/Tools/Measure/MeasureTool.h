/*
 * MeasureTool.h
 *
 *  Created on: Dec 11, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ESPINA_MEASURE_TOOL_H_
#define ESPINA_MEASURE_TOOL_H_

// ESPINA
#include <GUI/View/Widgets/Measures/MeasureWidget.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Support/Tool.h>
#include <Support/ViewManager.h>

class QAction;

namespace ESPINA
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
    EspinaWidgetSPtr m_widget;
    EventHandlerSPtr m_handler;
    ViewManagerSPtr  m_viewManager;
    QAction         *m_action;
  };

  using MeasureToolPtr  = MeasureTool *;
  using MeasureToolSPtr = std::shared_ptr<MeasureTool>;

} // namespace ESPINA

#endif /* MEASURETOOL_H_ */
