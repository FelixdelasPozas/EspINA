/*
 * ZoomTool.h
 *
 *  Created on: Nov 14, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef ESPINA_ZOOM_AREA_H
#define ESPINA_ZOOM_AREA_H

// EspINA
#include <Support/EventHandler.h>
#include <Support/Tool.h>
#include <Support/ViewManager.h>
#include <GUI/View/Widgets/EspinaWidget.h>

class QCursor;

namespace EspINA
{
  class ZoomSelectionWidget;

  class ZoomArea
  : public Tool
  {
    Q_OBJECT
    public:
      /* \brief ZoomArea class constructor.
       * \param[in] viewManager Application view manager.
       */
      explicit ZoomArea(ViewManagerSPtr viewManager);

      /* \brief ZoomArea class destructor.
       *
       */
      virtual ~ZoomArea();

      /* \brief Returns if the tool is enabled.
       *
       */
      virtual bool enabled() const;

      /* \brief Enables/Disables the tool.
       *
       */
      virtual void setEnabled(bool value);

      /* \brief Returns the actions contained in this tool.
       *
       */
      virtual QList<QAction *> actions() const;

    public slots:
      /* \brief Initializes the tool (inserts the widget in the view manager and sets the event handler).
       *
       */
      void initTool(bool value);

    private:
      bool                 m_enabled;
      EspinaWidgetSPtr     m_widget;
      ViewManagerSPtr      m_viewManager;
      QAction             *m_zoomArea;
      EventHandlerSPtr     m_zoomHandler;
  };

  class ZoomEventHandler
  : public EventHandler
  {
    Q_OBJECT
    public:
      /* \brief ZoomEventHandler class constructor.
       *
       */
      explicit ZoomEventHandler(ZoomSelectionWidget *widget);

      /* \brief ZoomEventHandler class destructor.
       *
       */
      virtual ~ZoomEventHandler()
      {}

      /* \brief Implements EventHandler::setInUse.
       *
       */
      virtual void setInUse(bool value);

      /* \brief Implements EventHandler::filterEvent.
       *
       */
      virtual bool filterEvent(QEvent *e, RenderView*view = nullptr);

    signals:
      void eventHandlerInUse(bool);

    private:
      ZoomSelectionWidget *m_widget;
  };

  using ZoomAreaSPtr = std::shared_ptr<ZoomArea>;

} // namespace EspINA

#endif /* ZOOMTOOL_H_ */
