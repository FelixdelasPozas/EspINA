/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge PeÃ±a Pastor <email>

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


#include "CODEFilterInspector.h"
#include <Filters/MorphologicalEditionFilter.h>
#include <GUI/ViewManager.h>

// Qt
#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QUndoCommand>

using namespace EspINA;


class CODEModification
: public QUndoCommand
{
public:
  CODEModification(MorphologicalEditionFilter *filter,
                   int radius,
                   QUndoCommand *parent = NULL)
  : QUndoCommand(parent)
  , m_filter(filter)
  , m_radius(radius)
  {
    m_oldRadius = m_filter->radius();
  }

  virtual void redo()
  {
    m_filter->setRadius(m_radius);

    update();
  }
  virtual void undo()
  {
    m_filter->setRadius(m_oldRadius);

    update();
  }

private:
  void update()
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_filter->update();
    QApplication::restoreOverrideCursor();
  }

private:
  MorphologicalEditionFilter *m_filter;

  int m_radius, m_oldRadius;
};


//----------------------------------------------------------------------------
CODESettings::CODESettings(QString title,
                           MorphologicalEditionFilter* filter,
                           QUndoStack  *undoStack,
                           ViewManager *viewManager)
: QWidget()
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_filter(filter)
{
  QGroupBox *group = new QGroupBox(title, this);
  QLabel *label = new QLabel(tr("Radius"));
  m_spinbox = new QSpinBox();
  m_spinbox->setMinimum(0);
  m_spinbox->setMaximum(100);
  m_spinbox->setValue(filter->radius());

  QHBoxLayout *groupLayout = new QHBoxLayout();
  groupLayout->addWidget(label);
  groupLayout->addWidget(m_spinbox);
  group->setLayout(groupLayout);

  QPushButton *modifyButton = new QPushButton(tr("Modify"));
  QHBoxLayout *modifyLayout = new QHBoxLayout();
  modifyLayout->addStretch();
  modifyLayout->addWidget(modifyButton);
  modifyLayout->addStretch();

  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->addWidget(group);
  mainLayout->addLayout(modifyLayout);
  mainLayout->addStretch();
  setLayout(mainLayout);

  connect(modifyButton, SIGNAL(clicked(bool)),
          this, SLOT(modifyFilter()));
  connect(m_filter, SIGNAL(modified(ModelItemPtr)),
          this, SLOT(updateWidget()));

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  setMinimumWidth(150);
}

//----------------------------------------------------------------------------
CODESettings::~CODESettings()
{

}

//----------------------------------------------------------------------------
void CODESettings::modifyFilter()
{
  m_undoStack->beginMacro("Modify Radius");
  {
    m_undoStack->push(new CODEModification(m_filter, m_spinbox->value()));
  }
  m_undoStack->endMacro();
}

//----------------------------------------------------------------------------
void CODESettings::updateWidget()
{
  m_spinbox->setValue(m_filter->radius());
}

//----------------------------------------------------------------------------
CODEFilterInspector::CODEFilterInspector(QString title, MorphologicalEditionFilter *filter)
: m_title(title)
, m_filter(filter)
{
}

//----------------------------------------------------------------------------
QWidget *CODEFilterInspector::createWidget(QUndoStack *stack, ViewManager *viewManager)
{
  return new CODESettings(m_title, m_filter, stack, viewManager);
}
