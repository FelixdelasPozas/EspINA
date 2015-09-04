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
#include <GUI/Widgets/ActionSelector.h>
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
  class SplitTool
  : public Support::Widgets::EditTool
  {
    Q_OBJECT

    class SplitFilterFactory
    : public FilterFactory
    {
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

    private:
      mutable DataFactorySPtr m_dataFactory;
    };

  public:
    /** \brief SplitTool class constructor.
     * \param[in] context ESPINA context
     *
     */
    SplitTool(Support::Context &context);

    /** \brief SplitTool class virtual destructor.
     *
     */
    virtual ~SplitTool();

  signals:
    void splittingStopped();

  private:
    void initSplitWidgets();

    void showCuttingPlane();

    void hideCuttingPlane();

  public slots:
    /** \brief Helper method called by the widgets on creation.
     *
     */
    void onWidgetCreated(PlanarSplitWidgetPtr widget);

    /** \brief Helper method called by the widgets on destruction.
     *
     */
    void onWidgetDestroyed(PlanarSplitWidgetPtr widget);

    /** \brief Helper method called by the widget that has finished defining the splitting plane.
     *
     */
    void onSplittingPlaneDefined(PlanarSplitWidgetPtr widget);

    /** \brief Disables the widget when the selection changes.
     *
     */
    void onSelectionChanged();

  private slots:
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

  private:
    using TemporalPrototypesSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;

    struct Data
    {
      FilterSPtr              adapter;
      SegmentationAdapterSPtr segmentation;

      Data(FilterSPtr adapterP, SegmentationAdapterSPtr segmentationP)
      : adapter{adapterP}, segmentation{segmentationP}
      {};

      Data(): adapter{nullptr}, segmentation{nullptr}
      {};
    };

    QPushButton *m_apply;

    PlanarSplitEventHandlerSPtr  m_handler;
    QMap<FilterPtr, struct Data> m_executingTasks;
    TemporalPrototypesSPtr       m_factory;
    QList<PlanarSplitWidgetPtr>  m_splitWidgets;
    vtkSmartPointer<vtkPlane>    m_splitPlane;
  };

  using SplitToolPtr  = SplitTool *;
  using SplitToolSPtr = std::shared_ptr<SplitTool>;

} // namespace ESPINA

#endif // ESPINA_SPLIT_TOOL_H_
