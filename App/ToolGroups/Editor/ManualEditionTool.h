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

#ifndef ESPINA_MANUAL_EDITION_TOOL_H_
#define ESPINA_MANUAL_EDITION_TOOL_H_

#include <Core/Factory/FilterFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <Support/Tool.h>

#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Selectors/Selector.h>
#include <Support/ViewManager.h>
#include <GUI/Selectors/BrushSelector.h>
#include <App/Tools/Brushes/CircularBrushSelector.h>
#include <App/Tools/Brushes/SphericalBrushSelector.h>
#include <GUI/Widgets/CategorySelector.h>

#include <QUndoStack>

class QAction;

namespace EspINA
{
  class SliderAction;
  
  class ManualEditionTool
  : public Tool
  {
    class ManualFilterFactory
    : public FilterFactory
    {
      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

      virtual FilterTypeList providedFilters() const;

    private:
      mutable FetchBehaviourSPtr m_fetchBehaviour;
    };

    Q_OBJECT
    public:
      ManualEditionTool(ModelAdapterSPtr model,
                        ModelFactorySPtr factory,
                        ViewManagerSPtr  viewManager,
                        QUndoStack      *undoStack);
      virtual ~ManualEditionTool();

      virtual void setEnabled(bool value);

      virtual bool enabled() const;

      virtual QList<QAction *> actions() const;

      virtual void abortOperation();
    signals:
      void stopDrawing();
      void brushModeChanged(BrushSelector::BrushMode);

    public slots:
      void drawStroke(ViewItemAdapterPtr, Selector::WorldRegion, Nm, Plane);
      void radiusChanged(int);
      void drawingModeChanged(bool);

    private slots:
      void changeSelector(QAction *);
      void changeRadius(int);
      void changeOpacity(int);
      void selectorInUse(bool);
      void unsetSelector();
      void categoryChanged(CategoryAdapterSPtr category);

    private:
      ModelAdapterSPtr m_model;
      ModelFactorySPtr m_factory;
      ViewManagerSPtr  m_viewManager;
      QUndoStack      *m_undoStack;

      FilterFactorySPtr  m_filterFactory;

      CircularBrushSelectorSPtr  m_circularBrushSelector;
      SphericalBrushSelectorSPtr m_sphericalBrushSelector;

      BrushSelectorSPtr m_currentSelector;
      ActionSelector   *m_drawToolSelector;
      CategorySelector *m_categorySelector;
      QMap<QAction *, SelectorSPtr> m_drawTools;

      SliderAction *m_radiusWidget;
      SliderAction *m_opacityWidget;

      QAction *m_discTool;
      QAction *m_sphereTool;

      bool m_enabled;
  };

  using ManualEditionToolPtr = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace EspINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_
