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

#ifndef ESPINA_SPLIT_TOOL_H_
#define ESPINA_SPLIT_TOOL_H_

// EspINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget.h>
#include <GUI/Widgets/ActionSelector.h>
#include <Support/Tool.h>
#include <Support/ViewManager.h>

// Qt
#include <QUndoStack>

class QAction;

namespace EspINA
{
  class SplitToolEventHandler;
  using SplitToolEventHandlerSPtr = std::shared_ptr<SplitToolEventHandler>;
  
  class SplitTool
  : public Tool
  {
    Q_OBJECT

    class SplitFilterFactory
    : public FilterFactory
    {
        virtual FilterTypeList providedFilters() const;
        virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

      private:
        mutable FetchBehaviourSPtr m_fetchBehaviour;
    };


    public:
      SplitTool(ModelAdapterSPtr model,
                ModelFactorySPtr factory,
                ViewManagerSPtr  viewManager,
                QUndoStack      *undoStack);
      virtual ~SplitTool();

      virtual void setEnabled(bool value);

      virtual bool enabled() const;

      virtual QList<QAction *> actions() const;

      virtual void abortOperation()
      { initTool(false); }

    signals:
      void splittingStopped();

    public slots:
      void initTool(bool);
      void applyCurrentState();
      void createSegmentations();

      void stopSplitting()
      { initTool(false); }

    private:
      struct Data
      {
        FilterAdapterSPtr adapter;
        SegmentationAdapterSPtr segmentation;

        Data(FilterAdapterSPtr adapterP, SegmentationAdapterSPtr segmentationP): adapter{adapterP}, segmentation{segmentationP} {};
        Data(): adapter{nullptr}, segmentation{nullptr} {};
      };

      void splitSegmentation();

      QAction *m_planarSplitAction;
      QAction *m_applyButton;

      ModelAdapterSPtr m_model;
      ModelFactorySPtr m_factory;
      ViewManagerSPtr  m_viewManager;
      QUndoStack      *m_undoStack;

      bool m_enabled;
      EspinaWidgetSPtr m_widget;
      SplitToolEventHandlerSPtr m_handler;
      QMap<FilterAdapterPtr, struct Data> m_executingTasks;
  };

  using SplitToolPtr  = SplitTool *;
  using SplitToolSPtr = std::shared_ptr<SplitTool>;

  class SplitToolEventHandler
  : public EventHandler
  {
    public:
      /* \brief SplitToolEventHandler class constructor.
       *
       */
      SplitToolEventHandler()
      {}

      /* \brief SplitToolEventHandler class destructor.
       *
       */
      ~SplitToolEventHandler()
      {}

      /* \brief Implements EventHandler::cursor().
       *
       */
      QCursor cursor() const
      { return QCursor(Qt::CrossCursor); }

      /* \brief Implements EventHandler::setInUse.
       *
       */
      virtual void setInUse(bool value)
      { m_inUse = value; }

      /* \brief Implements EventHandler::filterEvent.
       *
       */
      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr);
  };

} // namespace EspINA

#endif // ESPINA_SPLIT_TOOL_H_
