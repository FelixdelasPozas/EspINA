/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_RECTANGULAR_ROI_H
#define ESPINA_RECTANGULAR_ROI_H

#include <GUI/Selectors/Selector.h>
#include <GUI/Selectors/PixelSelector.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

namespace EspINA
{
  class RectangularRegion;
  class RectangularRegionSliceSelector;
  class ViewManager;

  class RectangularROI
  : public QObject
  {
  public:
    class ROISettings;
    class ROISettingsPanel;

    Q_OBJECT
  public:
    explicit RectangularROI(ModelAdapterSPtr model,
                            ViewManagerSPtr viewManager);
    virtual ~RectangularROI();

    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent* e, RenderView* view = nullptr);
    virtual void setInUse(bool value);
    virtual void setEnabled(bool enable);
    virtual bool enabled() const {return m_enabled;}

  private slots:
    void defineVOI(Selector::Selection);

  signals:
    void voiDeactivated();

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;

    bool m_inUse;
    bool m_enabled;

    PixelSelector      m_selector;
    RectangularRegion *m_widget;
    Bounds             m_bounds;
    RectangularRegionSliceSelector *m_sliceSelector;

    ROISettings          *m_settings;
    ROISettingsPanel     *m_settingsPanel;
  };

  using RectangularROIPtr  = RectangularROI *;
  using RectangularROISPtr = std::shared_ptr<RectangularROI>;

} // namespace EspINA

#endif // ESPINA_RECTANGULAR_ROI_H
