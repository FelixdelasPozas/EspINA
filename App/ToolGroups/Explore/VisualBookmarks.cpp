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
#include <ToolGroups/Explore/VisualBookmarks.h>

// Qt
#include <QPushButton>
#include <QComboBox>
#include <QDateTime>
#include <QInputDialog>

using namespace ESPINA;

//-----------------------------------------------------------------------------
VisualBookmarks::VisualBookmarks(Support::Context& context, QList<RenderView*> views)
: ProgressTool(":/espina/visualBookmarks.svg", tr("Visual Bookmarks"), context)
, m_views     {views}
{
  setCheckable(true);
  setExclusive(false);

  initWidgets();
}

//-----------------------------------------------------------------------------
VisualBookmarks::~VisualBookmarks()
{
}

//-----------------------------------------------------------------------------
void VisualBookmarks::abortOperation()
{
  setChecked(false);
}

//-----------------------------------------------------------------------------
void VisualBookmarks::add()
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
void VisualBookmarks::remove()
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
void VisualBookmarks::apply()
{
  auto bookmark = m_bookmarks.at(m_combobox->currentIndex());

  for(auto view: m_views)
  {
    view->setCameraState(bookmark.states[view->viewName()]);
  }
}

//-----------------------------------------------------------------------------
void VisualBookmarks::clear()
{
  setChecked(false);

  resetComboBox();
  m_bookmarks.clear();

  m_remove->setEnabled(false);
  m_apply ->setEnabled(false);
}

//-----------------------------------------------------------------------------
void VisualBookmarks::initWidgets()
{
  m_add    = GUI::Widgets::Styles::createToolButton(":/espina/visualBookmarks_add.svg", tr("Add current visual state as a bookmark"));

  m_remove = GUI::Widgets::Styles::createToolButton(":/espina/visualBookmarks_remove.svg", tr("Remove current bookmark"));
  m_remove->setEnabled(false);

  m_apply  = GUI::Widgets::Styles::createToolButton(":/espina/visualBookmarks_apply.svg", tr("Apply current bookmark"));
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
void VisualBookmarks::resetComboBox()
{
  m_combobox->clear();
  m_combobox->insertItem(0, tr("No bookmarks"));
  m_combobox->setCurrentIndex(0);
}
