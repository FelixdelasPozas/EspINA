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


#ifndef RECTANGULARVOI_H
#define RECTANGULARVOI_H

#include <GUI/Tools/IVOI.h>
#include <GUI/Pickers/ISelector.h>
#include <GUI/Pickers/PixelSelector.h>
#include <GUI/ISettingsPanel.h>
#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  class RectangularRegion;
  class RectangularRegionSliceSelector;
  class ViewManager;

  class RectangularVOI
  : public IVOI
  {
    class Settings;
    class SettingsPanel;

    Q_OBJECT
  public:
    explicit RectangularVOI(EspinaModel *model,
                            ViewManager *viewManager);
    virtual ~RectangularVOI();

    virtual QCursor cursor() const;
    virtual bool filterEvent(QEvent* e, EspinaRenderView* view = 0);
    virtual void setInUse(bool value);
    virtual void setEnabled(bool enable);
    virtual bool enabled() const {return m_enabled;}

    virtual IVOI::Region region();

  private slots:
    void defineVOI(ISelector::PickList channels);

  signals:
    void voiDeactivated();

  private:
    EspinaModel *m_model;
    ViewManager *m_viewManager;

    bool m_inUse;
    bool m_enabled;

    PixelSelector      m_picker;
    RectangularRegion *m_widget;
    double             m_bounds[6];
    RectangularRegionSliceSelector *m_sliceSelector;

    Settings          *m_settings;
    ISettingsPanelPrototype m_settingsPanel;
  };

  typedef boost::shared_ptr<RectangularVOI> RectangularVOISPtr;

} // namespace EspINA

#endif // RECTANGULARVOI_H
