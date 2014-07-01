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

#ifndef ESPINA_MANUAL_EDITION_TOOL_H_
#define ESPINA_MANUAL_EDITION_TOOL_H_

#include <GUI/Model/ModelAdapter.h>
#include <Support/Tool.h>

#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Selectors/Selector.h>
#include <Support/ViewManager.h>
#include <GUI/Selectors/BrushSelector.h>
#include <App/Tools/Brushes/CircularBrushSelector.h>
#include <App/Tools/Brushes/SphericalBrushSelector.h>
#include <GUI/Widgets/CategorySelector.h>

class QAction;

namespace EspINA
{
  class SliderAction;
  
  class ManualEditionTool
  : public Tool
  {
    Q_OBJECT
  public:
    ManualEditionTool(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager);
    virtual ~ManualEditionTool();

    virtual void setEnabled(bool value);

    virtual bool enabled() const;

    virtual QList<QAction *> actions() const;

    virtual void abortOperation();

    void showOpacityControls(bool value)
    { m_showOpacityControls = value; }

    void showRadiusControls(bool value)
    { m_showRadiusControls = value; }

    void showCategoryControls(bool value)
    { m_showCategoryControls = value; }

    bool opacityControls()
    { return m_showOpacityControls; }

    bool radiusControls()
    { return m_showRadiusControls; }

    bool categoryControls()
    { return m_showCategoryControls; }

    void setPencil2DIcon(QIcon icon)
    { m_discTool->setIcon(icon); }

    void setPencil3DIcon(QIcon icon)
    { m_sphereTool->setIcon(icon); }

    void setPencil2DText(QString text)
    { m_discTool->setText(text); }

    void setPencil3DText(QString text)
    { m_sphereTool->setText(text); }

  signals:
    void stopDrawing();
    void brushModeChanged(BrushSelector::BrushMode);
    void stroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char>);

  public slots:
    virtual void drawStroke(Selector::Selection);
    virtual void radiusChanged(int);
    virtual void drawingModeChanged(bool);
    virtual void updateReferenceItem();

  protected slots:
    virtual void changeSelector(QAction *);
    virtual void changeRadius(int);
    virtual void changeOpacity(int);
    virtual void selectorInUse(bool);
    virtual void unsetSelector();
    virtual void categoryChanged(CategoryAdapterSPtr category);

  protected:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;

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

    bool m_showOpacityControls;
    bool m_showRadiusControls;
    bool m_showCategoryControls;

    bool m_enabled;
  };

  using ManualEditionToolPtr = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace EspINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_
