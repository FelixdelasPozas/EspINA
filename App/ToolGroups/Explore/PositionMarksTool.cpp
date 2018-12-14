/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/ToolButton.h>
#include <ToolGroups/Explore/PositionMarksTool.h>

// Qt
#include <QComboBox>
#include <QDateTime>
#include <QInputDialog>

using namespace ESPINA;
using namespace ESPINA::Support::Widgets;

const QString PLANE       = "View's plane";
const QString SLICE       = "View's slice";
const QString POSITION    = "Camera position";
const QString FOCAL_POINT = "Focal point";
const QString UP_VECTOR   = "Up vector";
const QString HEIGHT      = "Camera height";

//-----------------------------------------------------------------------------
PositionMarksTool::PositionMarksTool(Support::Context& context, QList<RenderView*> views)
: ProgressTool("PositionMarks", ":/espina/position_marks.svg", tr("Position marks"), context)
, m_views     {views}
{
  setCheckable(true);
  setExclusive(false);

  initWidgets();
}

//-----------------------------------------------------------------------------
PositionMarksTool::~PositionMarksTool()
{
}

//-----------------------------------------------------------------------------
void PositionMarksTool::abortOperation()
{
  setChecked(false);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::saveSettings(std::shared_ptr<QSettings> settings)
{
  for(auto bookmark: m_bookmarks)
  {
    settings->beginGroup(bookmark.id);
    for(auto view: bookmark.states.keys())
    {
      auto data = bookmark.states[view];

      settings->beginGroup(view);

      settings->setValue(PLANE,       normalCoordinateIndex(data.plane));
      settings->setValue(SLICE,       data.slice);
      settings->setValue(HEIGHT,      data.heightLength);
      settings->setValue(POSITION,    data.cameraPosition.toString());
      settings->setValue(UP_VECTOR,   data.upVector.toString());
      settings->setValue(FOCAL_POINT, data.focalPoint.toString());

      settings->endGroup();
    }

    settings->endGroup();
  }
}

//-----------------------------------------------------------------------------
void PositionMarksTool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_bookmarks.clear();

  for(auto id: settings->childGroups())
  {
    struct CameraPositions bookmark;

    bookmark.id = id;

    settings->beginGroup(id);

    for(auto view: settings->childGroups())
    {
      settings->beginGroup(view);

      RenderView::CameraState state;
      state.plane = toPlane(settings->value(PLANE, 3).toInt());
      state.slice = settings->value(SLICE,0).toInt();
      state.heightLength = settings->value(HEIGHT,0).toDouble();

      auto positionString = settings->value(POSITION, NmVector3().toString()).toString();
      state.cameraPosition = NmVector3(positionString);

      auto upVectorString = settings->value(UP_VECTOR, NmVector3().toString()).toString();
      state.upVector = NmVector3(upVectorString);

      auto focalPointString = settings->value(FOCAL_POINT, NmVector3().toString()).toString();
      state.focalPoint = NmVector3(focalPointString);

      settings->endGroup();

      bookmark.states.insert(view, state);
    }

    settings->endGroup();

    m_bookmarks << bookmark;
  }

  auto enabled = !m_bookmarks.isEmpty();

  if(!enabled)
  {
    resetComboBox();
  }
  else
  {
    m_combobox->clear();

    for(auto bookmark: m_bookmarks)
    {
      m_combobox->insertItem(m_combobox->count(), bookmark.id);
    }

    m_combobox->setCurrentIndex(m_combobox->count()-1);
  }

  m_remove->setEnabled(enabled);
  m_apply ->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::add()
{
  CameraPositions state;
  QDateTime time = QDateTime::currentDateTime();
  state.id = time.toString();

  bool ok;
  QString text = QInputDialog::getText(m_add,
                                       tr("Enter name"),
                                       tr("Name:"),
                                       QLineEdit::Normal,
                                       state.id, &ok);

  if (!(ok && !text.isEmpty())) return;

  state.id = text;

  for(auto view: m_views)
  {
    state.states.insert(view->viewName(), view->cameraState());
  }

  m_bookmarks << state;

  if(m_bookmarks.size() == 1)
  {
    m_combobox->clear();
  }
  m_combobox->insertItem(m_combobox->count(), state.id);
  m_combobox->setCurrentIndex(m_combobox->count()-1);

  m_remove->setEnabled(true);
  m_apply ->setEnabled(true);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::remove()
{
  auto index = m_combobox->currentIndex();
  m_combobox->removeItem(index);
  m_bookmarks.removeAt(index);

  auto enabled = !m_bookmarks.isEmpty();

  if(!enabled)
  {
    resetComboBox();
  }

  m_remove->setEnabled(enabled);
  m_apply ->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::apply()
{
  auto bookmark = m_bookmarks.at(m_combobox->currentIndex());

  NmVector3 crosshair;
  auto spacing = getContext().viewState().coordinateSystem()->resolution();

  for(auto view: m_views)
  {
    view->setCameraState(bookmark.states[view->viewName()]);
    auto plane = bookmark.states[view->viewName()].plane;
    if(plane != Plane::UNDEFINED)
    {
      auto index = normalCoordinateIndex(plane);
      crosshair[index] = bookmark.states[view->viewName()].slice * spacing[index];
    }
  }

  getContext().viewState().setCrosshair(crosshair);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::clear()
{
  setChecked(false);

  resetComboBox();
  m_bookmarks.clear();

  m_remove->setEnabled(false);
  m_apply ->setEnabled(false);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::initWidgets()
{
  m_add = GUI::Widgets::Styles::createToolButton(":/espina/visualBookmarks_add.svg", tr("Add position mark"));

  m_remove = GUI::Widgets::Styles::createToolButton(":/espina/visualBookmarks_remove.svg", tr("Remove position mark"));
  m_remove->setEnabled(false);

  m_apply  = GUI::Widgets::Styles::createToolButton(":/espina/visualBookmarks_apply.svg", tr("Restore position mark"));
  m_apply->setEnabled(false);

  m_combobox = new QComboBox();
  m_combobox->setFixedWidth(200);
  resetComboBox();

  connect(m_add,    SIGNAL(clicked(bool)),
          this,     SLOT(add()));

  connect(m_remove, SIGNAL(clicked(bool)),
          this,     SLOT(remove()));

  connect(m_apply,  SIGNAL(clicked(bool)),
          this,     SLOT(apply()));

  addSettingsWidget(m_add);
  addSettingsWidget(m_remove);
  addSettingsWidget(m_combobox);
  addSettingsWidget(m_apply);
}

//-----------------------------------------------------------------------------
void PositionMarksTool::resetComboBox()
{
  m_combobox->clear();
  m_combobox->insertItem(0, tr("No position marks"));
  m_combobox->setCurrentIndex(0);
}
