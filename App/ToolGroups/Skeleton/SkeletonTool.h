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
#include <Support/ViewManager.h>
#include <Support/Widgets/Tool.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Qt
#include <QAction>

class vtkPolyData;
class QUndoStack;

namespace ESPINA
{
  class CategorySelector;
  class SpinBoxAction;

  class SourceFilterFactory
  : public FilterFactory
  {
    virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);
    virtual FilterTypeList providedFilters() const;

  private:
    mutable DataFactorySPtr m_fetchBehaviour;
  };

  class EspinaGUI_EXPORT SkeletonTool
  : public Tool
  {
    Q_OBJECT
    public:
      /** \brief SkeletonTool class constructor.
       * \param[in] model model adapter smart pointer.
       * \param[in] factory model factory smart pointer.
       * \param[in] viewManager view manager smart pointer.
       * \param[in] undoStack QUndoStack object raw pointer.
       *
       */
      SkeletonTool(ModelAdapterSPtr model, ModelFactorySPtr factory, ViewManagerSPtr viewManager, QUndoStack *undoStack);

      /** \brief SkeletonTool class virtual destructor.
       *
       */
      virtual ~SkeletonTool();

      virtual void setEnabled(bool value);

      virtual bool enabled() const
      { return m_enabled; }

      virtual QList<QAction *> actions() const;

      /** \brief Returns the Skeleton created by the user.
       *
       */
      vtkSmartPointer<vtkPolyData> getSkeleton()
      {
        return m_skeleton; m_skeleton = nullptr;
      }

      /** \brief Returns the category of the category selector of the tool.
       *
       */
      CategoryAdapterSPtr getSelectedCategory()
      { return m_itemCategory; }

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
      /** \brief Helper method to create a segmentation and assigns a skeleton output to it.
       *
       */
      void createSegmentation();

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

      /** \brief Removes the widget when the event handler is turned off
       * by another event handler.
       *
       */
      void eventHandlerToogled(bool toggled);

    private:
      /** \brief Helper method to manage the visibility of widgets.
       * \param[in] value true to set visible false otherwise.
       *
       */
      void setControlsVisibility(bool value);

      /** \brief Updates the ViewItem selected to use the spacing and set the tolerance.
       *
       */
      void updateReferenceItem();

      ViewManagerSPtr   m_vm;
      ModelAdapterSPtr  m_model;
      ModelFactorySPtr  m_factory;
      QUndoStack       *m_undoStack;
      bool              m_enabled;
      CategorySelector *m_categorySelector;
      EventHandlerSPtr  m_handler;
      QAction          *m_action;
      EspinaWidgetSPtr  m_widget;

      // widget's return values
      SegmentationAdapterPtr       m_item;
      CategoryAdapterSPtr          m_itemCategory;
      vtkSmartPointer<vtkPolyData> m_skeleton;
  };

  using SkeletonToolPtr  = SkeletonTool *;
  using SkeletonToolSPtr = std::shared_ptr<SkeletonTool>;

} // namespace EspINA

#endif // ESPINA_SKELETON_TOOL_H_
