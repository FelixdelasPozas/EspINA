/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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

#include "CountingRegion.h"
#include "ui_CountingRegion.h"
#include <common/EspinaCore.h>
#include "regions/RectangularRegion.h"

// #include "CountingRegionExtension.h"

const int ADAPTIVE = 0;
const int RECTANGULAR = 1;

//------------------------------------------------------------------------
class CountingRegion::GUI
: public QWidget
, public Ui::CountingRegion
{
public:
  explicit GUI(); 

private:
  bool eventFilter(QObject *object, QEvent *event);
};

//------------------------------------------------------------------------
CountingRegion::GUI::GUI()
{
  setupUi(this);

  leftMargin->installEventFilter(this);
  rightMargin->installEventFilter(this);
  topMargin->installEventFilter(this);
  bottomMargin->installEventFilter(this);
  lowerSlice->installEventFilter(this);
  upperSlice->installEventFilter(this);
}

//------------------------------------------------------------------------
bool CountingRegion::GUI::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::FocusIn)
  {
    if (object == leftMargin)
      preview->setPixmap(QPixmap(":/espina/left.png"));
    else if (object == rightMargin)
      preview->setPixmap(QPixmap(":/espina/right.png"));
    else if (object == topMargin)
      preview->setPixmap(QPixmap(":/espina/top.png"));
    else if (object == bottomMargin)
      preview->setPixmap(QPixmap(":/espina/bottom.png"));
    else if (object == upperSlice)
      preview->setPixmap(QPixmap(":/espina/upper.png"));
    else if (object == lowerSlice)
      preview->setPixmap(QPixmap(":/espina/lower.png"));

  }else if (event->type() == QEvent::FocusOut)
  {
    preview->setPixmap(QPixmap(":/espina/allPlanes.png"));
  }
  return QObject::eventFilter(object, event);
}


const QString CountingRegion::ID = "CountingRegionExtension";

//------------------------------------------------------------------------
CountingRegion::CountingRegion(QWidget * parent)
: EspinaDockWidget(parent)
, m_gui(new GUI())
// , m_focusedSample(NULL)
// , m_model(0,5)
// , m_parentItem(NULL)
{
  setObjectName("CountingRegionDock");
  setWindowTitle(tr("Counting Region"));
  setWidget(m_gui);

  m_gui->regionView->setModel(&m_regionModel);
  m_espinaModel = EspinaCore::instance()->model();

//   connect(regionView, SIGNAL(clicked(QModelIndex)),
// 	  this, SLOT(showInfo(QModelIndex)));

//   resetRegionsModel();

  connect(m_gui->createRegion, SIGNAL(clicked()),
          this, SLOT(createBoundingRegion()));
//   connect(removeRegion, SIGNAL(clicked()),
//           this, SLOT(removeBoundingRegion()));
//   connect(regionType, SIGNAL(currentIndexChanged(int)),
// 	  this, SLOT(regionTypeChanged(int)));

//   connect(EspINA::instance(), SIGNAL(focusSampleChanged(Sample*)),
//           this, SLOT(focusSampleChanged(Sample *)));
//   connect(&m_model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),
// 	  this, SLOT(visibilityModified()));

//   // Register Counting Brick extensions
//   SegmentationExtension segExt;
//   EspINAFactory::instance()->addSegmentationExtension(&segExt);
//   SampleExtension sampleExt;
//   EspINAFactory::instance()->addSampleExtension(&sampleExt);
//   RegionRenderer region;
//   EspINAFactory::instance()->addViewWidget(region.clone());
// //   EXTENSION_DEBUG(<< "Counting Region Panel created");
}

//------------------------------------------------------------------------
CountingRegion::~CountingRegion()
{
}

//------------------------------------------------------------------------
void CountingRegion::createBoundingRegion()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  int left   = m_gui->leftMargin->value();
  int top    = m_gui->topMargin->value();
  int upper  = m_gui->upperSlice->value();
  int right  = m_gui->rightMargin->value();
  int bottom = m_gui->bottomMargin->value();
  int lower  = m_gui->lowerSlice->value();

  if (m_gui->regionType->currentIndex() == ADAPTIVE)
  {
  } else if (m_gui->regionType->currentIndex() == RECTANGULAR)
  {
    new RectangularRegion(left,  top,    upper,
			  right, bottom, lower);
  } else
    Q_ASSERT(false);

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void CountingRegion::createRectangularRegion()
{
}

