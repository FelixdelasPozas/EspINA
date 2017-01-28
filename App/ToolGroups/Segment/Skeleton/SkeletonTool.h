/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_TOOL_H_
#define ESPINA_SKELETON_TOOL_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/EventHandler.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/Skeleton/SkeletonPointTracker.h>
#include <GUI/Widgets/CategorySelector.h>
#include <Support/Widgets/ProgressTool.h>
#include <Support/Context.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Qt
#include <QAction>

class vtkPolyData;
class QUndoStack;

namespace ESPINA
{
  namespace //unnamed namespace
  {
    namespace SkeletonNamespace = GUI::View::Widgets::Skeleton;
  }
  class DoubleSpinBoxAction;
  class SkeletonToolStatusAction;

  /** \class ManualFilterFactory
   * \brief Factory for SourceFilter filters.
   */
  class SkeletonFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type SKELETON_FILTER;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type &filter, SchedulerSPtr scheduler) const override;

      virtual FilterTypeList providedFilters() const override;

    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory for this factory. */
  };

  /** \class SkeletonTool
   * \brief Tool for skeleton segmentation and edition.
   *
   */
  class EspinaGUI_EXPORT SkeletonTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief SkeletonTool class constructor.
       * \param[in] context ESPINA context
       *
       */
      SkeletonTool(Support::Context& context);

      /** \brief SkeletonTool class virtual destructor.
       *
       */
      virtual ~SkeletonTool();

      /** \brief Returns the category of the category selector of the tool.
       *
       */
      CategoryAdapterSPtr getSelectedCategory()
      { return m_categorySelector->selectedCategory(); }

      /** \brief Returns the item the skeleton has been created for or a nullptr
       *  if there is no item (created a new one).
       */
      SegmentationAdapterPtr getSelectedItem()
      { return m_item; }

      /** \brief Aborts the current operation.
       *
       */
      void abortOperation()
      { initTool(false); };

    public slots:
      /** \brief Helper method to modify an existing skeleton of a segmentation.
       * \param[in] polyData smart pointer of the new vtkPolyData.
       *
       */
      void skeletonModification(vtkSmartPointer<vtkPolyData> polyData);

      /** \brief Helper method to update the representation in the widget if the data
       *  being edited changes (by undo/redo).
       *
       */
      void updateWidgetRepresentation();

    private slots:
      /** \brief Performs tool initialization/de-initialization.
       * \param[in] value, true to initialize and false otherwise.
       *
       */
      void initTool(bool value);

      /** \brief Updates the state of the tool depending on the current selection.
       *
       */
      void updateState();

      /** \brief Updates the widget with the new category properties.
       * \param[in] category CategoryAdapter smart pointer.
       *
       */
      void categoryChanged(CategoryAdapterSPtr category);

      /** \brief Updates the tolerance value in the widgets when the value in the spinbox changes.
       * \param[in] value new tolerance value.
       *
       */
      void toleranceValueChanged(double value);

      /** \brief Updates the widget if the item being modified is removed from the model (i.e. by undo).
       * \param[in] segmentations List of segmentation adapter smart pointers removed from the model.
       *
       */
      void checkItemRemoval(SegmentationAdapterSList segmentations);

    private:
      virtual void onToolGroupActivated();

      /** \brief Helper method to manage the visibility of widgets.
       * \param[in] value true to set visible false otherwise.
       *
       */
      void setControlsVisibility(bool value);

      /** \brief Updates the ViewItem selected to use the spacing and set the tolerance.
       *
       */
      void updateReferenceItem();

    private:
      GUI::Widgets::CategorySelector             *m_categorySelector; /** category selector widget.       */
      DoubleSpinBoxAction                        *m_toleranceWidget;  /** distance between points widget. */
      SkeletonNamespace::SkeletonPointTrackerSPtr m_eventHandler;     /** tool's event handler.           */
      SegmentationAdapterPtr                      m_item;             /** current element being created.  */

      // TODO: 27-05-2015 SkeletonTool/Widget refactorization
      //EspinaWidgetSPtr          m_widget;
  };

  using SkeletonToolPtr  = SkeletonTool *;
  using SkeletonToolSPtr = std::shared_ptr<SkeletonTool>;

} // namespace EspINA

#endif // ESPINA_SKELETON_TOOL_H_
