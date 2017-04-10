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

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitEventHandler.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget.h>
#include <Support/Widgets/EditTool.h>
#include <Support/Context.h>

// Qt
#include <QUndoStack>

// VTK
#include <vtkSmartPointer.h>

class QAction;
class vtkPlane;

using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;

namespace ESPINA
{
  /** \class SplitFilterFactory
   * \brief Factory for split filters.
   *
   */
  class SplitFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type SPLIT_FILTER;    /** split filter signature. */
      static const Filter::Type SPLIT_FILTER_V4; /** split filter old signature. */

      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;

    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory for split filters. */
  };

  /** \class SplitTool
   * \brief Tool for split filters.
   *
   */
  class SplitTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief SplitTool class constructor.
       * \param[in] context ESPINA context
       *
       */
      explicit SplitTool(Support::Context &context);

      /** \brief SplitTool class virtual destructor.
       *
       */
      virtual ~SplitTool();

    signals:
      void splittingStopped();

    private:
      /** \brief Initializes the split option widgets.
       *
       */
      void initSplitWidgets();

      /** \brief Shows the widgets on the views.
       *
       */
      void showCuttingPlane();

      /** \brief Hides the widgets from the views.
       *
       */
      void hideCuttingPlane();

    public slots:
      /** \brief Helper method called by the 2D widgets on creation.
       * \param[in] widget created 2D widget.
       *
       */
      void onWidgetCreated(TemporalRepresentation2DSPtr widget);

      /** \brief Helper method called by the 3D widgets on creation.
       * \param[in] widget created 3D widget.
       *
       */
      void onWidgetCreated(TemporalRepresentation3DSPtr widget);

      /** \brief Helper method called by the widgets on destruction.
       * \param[in] object pointer to the destroyed QObject.
       *
       */
      void onWidgetDestroyed(QObject *object);

      /** \brief Helper method called by the widget that has finished defining the splitting plane.
       * \param[in] widget planer split widget with the defined splitting plane.
       *
       */
      void onSplittingPlaneDefined(PlanarSplitWidgetPtr widget);

      /** \brief Disables the widget when the selection changes.
       *
       */
      void onSelectionChanged();

    private slots:
      /** \brief Shows/hides the splitting widget.
       * \param[in] enable true to enable and make visible and false otherwise.
       *
       */
      void toggleWidgetsVisibility(bool enable);

      /** \brief Splits the segmentation using the current state of the tool.
       *
       */
      void applyCurrentState();

      /** \brief Creates the segmentations and adds them to the model.
       *
       */
      void createSegmentations();

      /** \brief Stops current operation.
       *
       */
      void stopSplitting()
      { toggleWidgetsVisibility(false); }

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations);

      /** \brief Aborts the currently executing tasks.
       *
       */
      void abortTasks();

    private:
      using TemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;

      /** \struct Data
       * \brief Split filters required data for execution and results return.
       *
       */
      struct Data
      {
        FilterSPtr              filter;       /** filter . */
        SegmentationAdapterSPtr segmentation; /** segmentation to cut. */

        /** \brief Data constructor.
         * \param[in] adapter filter.
         * \param[in] segmentation segmentation to cut.
         *
         */
        Data(FilterSPtr filterP, SegmentationAdapterSPtr segmentationP)
        : filter{filterP}, segmentation{segmentationP}
        {};

        /** \brief Data empty constructor.
         *
         */
        Data()
        : filter{nullptr}, segmentation{nullptr}
        {};
      };

      QPushButton *m_apply; /** apply cutting button. */

      PlanarSplitEventHandlerSPtr  m_handler;        /** widget's event handler. */
      QMap<FilterPtr, struct Data> m_executingTasks; /** map of executing filters and it's data. */
      TemporalPrototypesSPtr       m_factory;        /** widget's factory. */
      QList<QObject *>             m_splitWidgets;   /** list of present widgets. */
      vtkSmartPointer<vtkPlane>    m_splitPlane;     /** user defined splitting plane. */
  };

  using SplitToolPtr  = SplitTool *;
  using SplitToolSPtr = std::shared_ptr<SplitTool>;

} // namespace ESPINA

#endif // ESPINA_SPLIT_TOOL_H_
