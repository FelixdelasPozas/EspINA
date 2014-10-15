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
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Qt
#include <QAction>

namespace ESPINA
{
  class CategorySelector;
  class SpinBoxAction;

  class EspinaGUI_EXPORT SkeletonTool
  : public Tool
  {
    Q_OBJECT
    public:
      /** \brief SkeletonTool class constructor.
       * \param[in] model model adapter smart pointer.
       * \param[in] viewManager view manager smart pointer.
       *
       */
      SkeletonTool(ModelAdapterSPtr model, ViewManagerSPtr viewManager);

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
      { return m_skeleton; m_skeleton = nullptr; }

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

    signals:
      void stoppedOperation();

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

      /** \brief Updates the internal tolerance value.
       * \param[in] value new tolerance value.
       *
       */
      void toleranceChanged(int value);

      /** \brief Updates the widget with the new category properties.
       * \param[in] category CategoryAdapter smart pointer.
       *
       */
      void categoryChanged(CategoryAdapterSPtr category);

    private:
      /** \brief Helper method to manage the visibility of widgets.
       * \param[in] value true to set visible false otherwise.
       *
       */
      void setControlsVisibility(bool value);

      ViewManagerSPtr   m_vm;
      bool              m_enabled;
      CategorySelector *m_categorySelector;
      SpinBoxAction    *m_toleranceBox;
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
