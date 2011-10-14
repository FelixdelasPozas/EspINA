/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "VesicleValidator.h"

#include "VesicleValidatorFilter.h"
#include "VesicleValidatorSettings.h"

#include <pixelSelector.h>

// Qt
#include <QHBoxLayout>
#include <QWidgetAction>
#include <QMessageBox>
#include <sample.h>
#include <RectangularVOI.h>
#include <espina.h>
#include <EspinaPluginManager.h>


//-----------------------------------------------------------------------------
VesicleValidator::VesicleValidator(QObject* parent)
: ISegmentationPlugin(parent)
, m_defineArea(new QToolButton())
, m_removeArea(new QToolButton())
, m_validateVesicles(new QToolButton())
, m_state(NONE)
, m_selector(new PixelSelector())
, m_activeValidator(NULL)
, m_settings(new VesicleValidatorSettings())
, m_SVA(NULL)
{
  buildUI();

  initSelector();
  
  SelectionManager *selMgr = SelectionManager::instance();
  connect(selMgr,SIGNAL(VOIChanged(IVOI*)),
	  this, SLOT(changeVOI(IVOI *)));
  
  EspinaPluginManager::instance()->registerFilter("VesicleValidator::VesicleValidatorFilter",this);
}

//-----------------------------------------------------------------------------
VesicleValidator::~VesicleValidator()
{
  foreach(VesicleValidatorFilter *validator, m_validators)
    delete validator;
  
  m_activeValidator = NULL;
  m_validators.clear();
}


//-----------------------------------------------------------------------------
void initButton(QToolButton *button, const QString &icon)
{
  button->setCheckable(true);
  button->setIcon(QIcon(icon));
  button->setIconSize(QSize(22,22));
  button->setAutoRaise(true);
  button->setEnabled(false);
}

//-----------------------------------------------------------------------------
void VesicleValidator::buildUI()
{
  initButton(m_defineArea,      ":/defineArea.svg");
  initButton(m_removeArea,      ":/removeArea.svg");
  initButton(m_validateVesicles,":/validateVesicle.svg");

  connect(m_defineArea, SIGNAL(toggled(bool)),
	  this, SLOT(defineSVAClicked(bool)));
  connect(m_removeArea, SIGNAL(toggled(bool)),
	  this, SLOT(removeSVAClicked(bool)));
  connect(m_validateVesicles, SIGNAL(toggled(bool)),
	  this, SLOT(validateVesiclesClicked(bool)));
  
  // Plugin's Widget Layout
  QHBoxLayout *layout = new QHBoxLayout();
  //layout->addWidget(m_defineArea);
  //layout->addWidget(m_removeArea);
  layout->addWidget(m_validateVesicles);
  
  QWidget *widget = new QWidget();
  widget->setLayout(layout);

  QWidgetAction *toolAction = new QWidgetAction(this);
  toolAction->setDefaultWidget(widget); 
}

//-----------------------------------------------------------------------------
void VesicleValidator::updateTaxonomy()
{
  EspINA *espina = EspINA::instance();
  TaxonomyNode *tax = espina->taxonomy();
  TaxonomyNode *vesicleTax = tax->getComponent("Vesicles");
  
  if (!vesicleTax)
  {
    espina->addTaxonomy("Vesicle",tax->getName());
    vesicleTax = tax->getComponent("Vesicle");
  }
  
  TaxonomyNode *likely = vesicleTax->getComponent("Likely");
  if (!likely)
  {
    espina->addTaxonomy("Likely",vesicleTax->getName());
    likely = vesicleTax->getComponent("Likely");
    likely->setColor(QColor(Qt::green));
  }
  
  TaxonomyNode *unlikely = vesicleTax->getComponent("Unlikely");
  if (!unlikely)
  {
    espina->addTaxonomy("Unlikely",vesicleTax->getName());
    unlikely = vesicleTax->getComponent("Unlikely");
    unlikely->setColor(QColor(Qt::red));
  }
}



//-----------------------------------------------------------------------------
void VesicleValidator::initSelector()
{
  m_selector->multiSelection = false;
  m_selector->filters << "EspINA_Sample";
  connect(m_selector, SIGNAL(selectionChanged(ISelectionHandler::MultiSelection)),
	  this, SLOT(handleSelection(ISelectionHandler::MultiSelection)));
  connect(m_selector, SIGNAL(selectionAborted()),
    this, SLOT(abortSelection()));
}


//-----------------------------------------------------------------------------
EspinaFilter* VesicleValidator::createFilter(QString filter, ITraceNode::Arguments& args)
{
  if (filter == "VesicleValidator::VesicleValidatorFilter")
  {
    VesicleValidatorFilter *validator = new VesicleValidatorFilter(args);
    m_validators.append(validator);
    return validator;
  }
  qWarning("VesicleValidator::createFilter: Error no such a Filter");
  return NULL;
}

//-----------------------------------------------------------------------------
void VesicleValidator::changeState(const STATE state)
{
  if (m_state == state)
    return;
  
  m_state = state;

  m_defineArea->setChecked(state   ==   DEFINING_SVA);
  m_removeArea->setChecked(state   ==   REMOVING_SVA);
  m_validateVesicles->setChecked(state == VALIDATING);

  switch (m_state)
  {
    case DEFINING_SVA:
      updateTaxonomy();
      SelectionManager::instance()->setSelectionHandler(
	m_selector,
	QCursor(QPixmap(":/defineArea.svg").scaled(32,32)));
      showSVAs();
      break;
    case REMOVING_SVA:
      SelectionManager::instance()->setSelectionHandler(
	m_selector,
	QCursor(QPixmap(":/removeArea.svg").scaled(32,32)));
      showSVAs();
      break;
    case VALIDATING:
      SelectionManager::instance()->setSelectionHandler(
	m_selector,
	Qt::CrossCursor);
      showActiveSVA();
      break;
    default:
      SelectionManager::instance()->setSelectionHandler(
	NULL,
	Qt::ArrowCursor);
      hideSVAs();
  };
}


//-----------------------------------------------------------------------------
void VesicleValidator::defineSVAClicked(bool active)
{
  if (active)
    changeState(DEFINING_SVA);
  else if (m_state == DEFINING_SVA)
    changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::defineSVA(const Point& pos, EspinaProduct *sample)
{
  double spacing[3];
  dynamic_cast<Sample *>(sample)->spacing(spacing);
  double bounds[6] = {
    (pos.x - m_settings->xSize())*spacing[0],
    (pos.x + m_settings->xSize())*spacing[0],
    (pos.y - m_settings->ySize())*spacing[1],
    (pos.y + m_settings->ySize())*spacing[1],
    (pos.z - m_settings->zSize())*spacing[2],
    (pos.z + m_settings->zSize())*spacing[2]};
  
//     m_activeValidator = new VesicleValidatorFilter(sample, bounds);
//   m_validators.append(m_activeValidator);
  changeState(VALIDATING);
}


//-----------------------------------------------------------------------------
void VesicleValidator::removeSVAClicked(bool active)
{
  if (active)
    changeState(REMOVING_SVA);
  else if (m_state == REMOVING_SVA)
    changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::removeSVA(const Point &pos)
{
  changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::validateVesiclesClicked(bool active)
{
  if (active)
//     if (m_activeValidator)
    if (m_SVA)
      changeState(VALIDATING);
    else
    {
      QMessageBox msg(QMessageBox::Information,
		      "Vesicle Validator Filter Information",
		      "There is no Segmentation's Vesicle Area selected. "
		      "Please, define a new area and then start validating."
		     );
      msg.exec();
      changeState(DEFINING_SVA);    
    }
  else if (m_state == VALIDATING)
    changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::validateVesicle( const ISelectionHandler::Selelection& selection)
{
  assert(m_SVA);
  
  Sample *sample = dynamic_cast<Sample*>(selection.second);
  assert(sample);
  
  assert(selection.first.size() == 1); // with one pixel
  Point pos = selection.first.first();
  double bounds[6];
  m_SVA->bounds(bounds);
  m_activeValidator = new VesicleValidatorFilter(sample, pos, bounds);
  m_validators.append(m_activeValidator);
  
//   m_activeValidator->validateVesicle(pos);
}

//-----------------------------------------------------------------------------
void VesicleValidator::showSVAs()
{

}

//-----------------------------------------------------------------------------
void VesicleValidator::hideSVAs()
{

}

//-----------------------------------------------------------------------------
void VesicleValidator::showActiveSVA()
{

}

//-----------------------------------------------------------------------------
void VesicleValidator::abortSelection()
{
  changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::changeVOI ( IVOI* voi )
{
  m_SVA = voi;
  if (m_SVA) 
  {
    updateTaxonomy();
  } else {
    m_activeValidator = NULL;
  }
  m_validateVesicles->setEnabled(m_SVA != NULL);
}


//-----------------------------------------------------------------------------
void VesicleValidator::handleSelection(const ISelectionHandler::MultiSelection& msel)
{
  assert( msel.size() == 1);
  ISelectionHandler::Selelection sel = msel.first();
//   EspinaProduct *sample = element.second;// Only one element is selected
//   assert(sample);
  
//   assert(element.first.size() == 1); // with one pixel
//   Point pos = element.first.first();
  
  switch (m_state)
  {
    case DEFINING_SVA:
//       defineSVA(pos, sample);
      break;
    case REMOVING_SVA:
//       removeSVA(pos);
      break;
    case VALIDATING:
      validateVesicle(sel);
      break;
    case NONE:
    default:
      assert(false);
  };
}


