/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DefaultViewSettingsPanel.h"

#include <QVBoxLayout>
#include <QGroupBox>

using namespace EspINA;

//-----------------------------------------------------------------------------
DefaultViewSettingsPanel::DefaultViewSettingsPanel(View2D* viewXY,
                                                   View2D* viewXZ,
                                                   View2D* viewYZ,
                                                   View3D* view3D,
                                                   RendererSList renderers)
: m_viewXY(viewXY)
, m_viewYZ(viewYZ)
, m_viewXZ(viewXZ)
, m_view3D(view3D)
, m_renderers(renderers)
{
  QVBoxLayout *layout = new QVBoxLayout();
  QGroupBox *group;
  QVBoxLayout *groupLayout;

  // Axial View
  m_panelXY = new View2DSettingsPanel(m_viewXY);
  group = new QGroupBox(m_panelXY->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_panelXY);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  // Coronal View
  m_panelXZ = new View2DSettingsPanel(m_viewXZ);
  group = new QGroupBox(m_panelXZ->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_panelXZ);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  // Sagittal View
  m_panelYZ = new View2DSettingsPanel(m_viewYZ);
  group = new QGroupBox(m_panelYZ->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_panelYZ);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  // 3D View
  m_panel3D = new View3DSettingsPanel(m_view3D, m_renderers);
  group = new QGroupBox(m_panel3D->shortDescription());
  groupLayout = new QVBoxLayout();
  groupLayout->addWidget(m_panel3D);
  group->setLayout(groupLayout);
  layout->addWidget(group);

  this->setLayout(layout);
}

//-----------------------------------------------------------------------------
void DefaultViewSettingsPanel::acceptChanges()
{
  m_panelXY->acceptChanges();
  m_panelXZ->acceptChanges();
  m_panelYZ->acceptChanges();
  m_panel3D->acceptChanges();
}

//-----------------------------------------------------------------------------
void DefaultViewSettingsPanel::rejectChanges()
{

}

//-----------------------------------------------------------------------------
bool DefaultViewSettingsPanel::modified() const
{
  return m_panelXY->modified()
      || m_panelXZ->modified()
      || m_panelYZ->modified()
      || m_panel3D->modified();
}

//-----------------------------------------------------------------------------
SettingsPanelPtr DefaultViewSettingsPanel::clone()
{
  return new DefaultViewSettingsPanel(m_viewXY, m_viewYZ, m_viewXZ, m_view3D, m_renderers);
}
