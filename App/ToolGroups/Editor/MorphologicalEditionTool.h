/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
#define ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_

#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>
#include <Support/Tool.h>

class QAction;
class QUndoStack;

namespace EspINA
{
  class SpinBoxAction;
  
  class MorphologicalEditionTool
  : public Tool
  {
    Q_OBJECT
    public:
      /** \brief Class constructor.
       *
       */
      MorphologicalEditionTool(ModelAdapterSPtr model,
                               ModelFactorySPtr factory,
                               ViewManagerSPtr  viewManager,
                               QUndoStack      *undoStack);

      /** \brief Class destructor.
       *
       */
      virtual ~MorphologicalEditionTool();

      /** \brief Tool method to enable/disable this tool.
       *
       */
      virtual void setEnabled(bool value);

      /** \brief Returns if the class is actually enabled.
       *
       */
      virtual bool enabled() const;

      /** \brief Returns the group of actions provided by the tool.
       *
       */
      virtual QList<QAction *> actions() const;

    public slots:
      /** \brief Merge selected segmentations.
       *
       */
      void mergeSegmentations();

      /** \brief Substract one segmentation from the other.
       *
       */
      void subtractSegmentations();

      /** \brief Erode the selected segmentation with the radius set on the associated QSpinBox.
       *
       */
      void erodeSegmentations();

      /** \brief Dilate the selected segmentation with the radius set on the associated QSpinBox.
       *
       */
      void dilateSegmentations();

      /** \brief Open the selected segmentation with the radius set on the associated QSpinBox.
       *
       */
      void openSegmentations();

      /** \brief Close the selected segmentation with the radius set on the associated QSpinBox.
       *
       */
      void closeSegmentations();

      /** \brief Fills all internals holes in the selected segmentation.
       *
       */
      void fillHoles();

    private:
      ModelAdapterSPtr m_model;
      ModelFactorySPtr m_factory;
      ViewManagerSPtr  m_viewManager;
      QUndoStack      *m_undoStack;

      QAction *m_addition;
      QAction *m_subtract;
      QAction *m_erode;
      QAction *m_dilate;
      QAction *m_open;
      QAction *m_close;
      QAction *m_fill;
      SpinBoxAction *m_dilateRadiusWidget;
      SpinBoxAction *m_erodeRadiusWidget;
      SpinBoxAction *m_openRadiusWidget;
      SpinBoxAction *m_closeRadiusWidget;

      bool m_enabled;
  };

  using MorphologicalEditionToolPtr  = MorphologicalEditionTool *;
  using MorphologicalEditionToolSPtr = std::shared_ptr<MorphologicalEditionTool>;

} // namespace EspINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
