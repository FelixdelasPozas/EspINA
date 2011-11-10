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
#include <RectangularVOI.h>

// Qt
#include <QHBoxLayout>
#include <QWidgetAction>
#include <QMessageBox>
#include <sample.h>
#include <RectangularVOI.h>
#include <espina.h>
#include <EspinaPluginManager.h>
#include <vtkSMProxy.h>

#include <QDebug>
#include <espinaMainWindow.h>
#include <QApplication>

bool VesicleValidator::Region::operator==(const VesicleValidator::Region& rhs)
{
  return (sva == rhs.sva && taxonomy == rhs.taxonomy);
}

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
, m_lastId(0)
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
//   button->setEnabled(false);
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
  layout->addWidget(m_defineArea);
  layout->addWidget(m_removeArea);
  layout->addWidget(m_validateVesicles);
  
  QWidget *widget = new QWidget();
  widget->setLayout(layout);

  QWidgetAction *toolAction = new QWidgetAction(this);
  toolAction->setDefaultWidget(widget); 
}

//-----------------------------------------------------------------------------
VesicleValidator::Region VesicleValidator::region(const QString& name) const
{
  Region region;
  foreach(region, m_regions)
  {
    if (region.taxonomy->name() == name)
      return region;
  }
    
  region.sva = NULL;
  region.taxonomy = NULL;
  return region;
}


//-----------------------------------------------------------------------------
void VesicleValidator::updateRegions()
{
  m_regions.clear();
  Taxonomy *tax = EspINA::instance()->taxonomy();
  TaxonomyNode *vesicles = tax->element(tax->name()+"/Vesicles");
  
  TaxonomyNode *taxonomy;
  foreach(taxonomy, vesicles->subElements())
  {
    if (!taxonomy->name().contains("Region"))
      continue;
    
    Region r = region(taxonomy->name());
    if (r.sva && r.taxonomy)
      continue;
    
    int taxId = taxonomy->name().section("Region",-1).toInt();
    if (taxId > m_lastId)
      m_lastId = taxId;
    r.taxonomy = taxonomy;
    r.sva = new RectangularVOI(false);
    QStringList savedBounds = taxonomy->property("SVA").toString().split(",");
    double bounds[6] = {savedBounds[0].toDouble(), savedBounds[1].toDouble(),
		        savedBounds[2].toDouble(), savedBounds[3].toDouble(),
		        savedBounds[4].toDouble(), savedBounds[5].toDouble()};
    r.sva->setDefaultBounds(bounds);
        
    m_regions.append(r);
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
      updateRegions();
      SelectionManager::instance()->setVOI(NULL);
      SelectionManager::instance()->setSelectionHandler(
	m_selector,
	QCursor(QPixmap(":/defineArea.svg").scaled(32,32)));
      showSVAs();
      break;
    case REMOVING_SVA:
      updateRegions();
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
      QApplication::restoreOverrideCursor();
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
VesicleValidator::Region VesicleValidator::searchSVG(const Point& pos, double spacing[3])
{
  Region region;
  foreach(region, m_regions)
  {
    // Use taxonomy region instead of voi region
    double bounds[6];
    QStringList regBounds = region.taxonomy->property("SVA").toString().split(",");
    for (int i=0; i < 6; i++)
      bounds[i] = regBounds[i].toDouble();
    
//     region.sva->bounds(bounds);
    double point[3];
    point[0] = pos.x*spacing[0];
    point[1] = pos.y*spacing[1];
    point[2] = pos.z*spacing[2];
    
    if ((bounds[0] <= point[0] && point[0] <= bounds[1]) &&
        (bounds[2] <= point[1] && point[1] <= bounds[3]) &&
        (bounds[4] <= point[2] && point[2] <= bounds[5]))
      return region;
  }
  
  region.sva = NULL;
  region.taxonomy = NULL;
  return region;
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
  
  Taxonomy *tax = EspINA::instance()->taxonomy();
//   tax->addElement("Vesicles/Region 1");
  TaxonomyNode *vesicles = tax->addElement("Vesicles");
  
  Region existingRegion = searchSVG(pos, spacing);
  if (existingRegion.sva && existingRegion.taxonomy)
  {
    m_SVA = existingRegion;
    double prevBounds[6];
    QStringList regBounds = existingRegion.taxonomy->property("SVA").toString().split(",");
    for (int i=0; i < 6; i++)
      prevBounds[i] = regBounds[i].toDouble();
//     m_SVA.sva->bounds(prevBounds);
    m_SVA.sva->setDefaultBounds(prevBounds);
    m_SVA.sva->resizeToDefaultSize();
    TaxonomyNode *likely = m_SVA.taxonomy->element("Likely");
    EspINA::instance()->setUserDefindedTaxonomy(likely->qualifiedName());
    connect(m_SVA.sva,SIGNAL(voiModified()),
	    this,SLOT(updateBounds()));
  }
  else
  {
    TaxonomyNode *region   = tax->addElement(QString("Region %1").arg(++m_lastId),
    vesicles->qualifiedName());
    region->addProperty("SVA",
    QString("%1,%2,%3,%4,%5,%6")
    .arg(bounds[0]).arg(bounds[1])
    .arg(bounds[2]).arg(bounds[3])
    .arg(bounds[4]).arg(bounds[5])
    );
    TaxonomyNode *likely   = tax->addElement("Likely",region->qualifiedName());
    TaxonomyNode *unlikely = tax->addElement("Unlikely",region->qualifiedName());
    likely->setColor(Qt::green);
    unlikely->setColor(Qt::red);
    EspINA::instance()->loadTaxonomy(tax);
    EspINA::instance()->setUserDefindedTaxonomy(likely->qualifiedName());
    
    m_SVA.sva = new RectangularVOI(false);
    m_SVA.sva->setDefaultBounds(bounds);
    m_SVA.taxonomy = region;
    m_regions.append(m_SVA);
    connect(m_SVA.sva,SIGNAL(voiModified()),
	    this,SLOT(updateBounds()));
  }
  
  changeState(NONE);
  SelectionManager::instance()->setVOI(m_SVA.sva);
}


//-----------------------------------------------------------------------------
void VesicleValidator::removeSVAClicked(bool active)
{
  if (active)
  {
    SelectionManager::instance()->setVOI(NULL);
    changeState(REMOVING_SVA);
  }
  else if (m_state == REMOVING_SVA)
    changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::removeSVA(const Point &pos, EspinaProduct *sample)
{
  double spacing[3];
  dynamic_cast<Sample *>(sample)->spacing(spacing);
  Region existingRegion = searchSVG(pos, spacing);
  if (existingRegion.sva && existingRegion.taxonomy)
  {
    EspINA *espina = EspINA::instance();
    EspinaMainWindow::instance()->clearSelection();
    Segmentation *seg;
    TaxonomyNode *subTax;
    foreach(subTax, existingRegion.taxonomy->subElements())
    {
      foreach(seg, espina->segmentations(subTax))
      {
	espina->removeSegmentation(seg);
      }
      espina->removeTaxonomy(subTax->qualifiedName());
    }
    espina->removeTaxonomy(existingRegion.taxonomy->qualifiedName());
    m_regions.remove(m_regions.indexOf(existingRegion));;
//     delete existingRegion.taxonomy;
  }
  
  changeState(NONE);
}

//-----------------------------------------------------------------------------
void VesicleValidator::validateVesiclesClicked(bool active)
{
  if (active)
//     if (m_activeValidator)
    if (m_SVA.sva)
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
  assert(m_SVA.sva);
  
  Sample *sample = dynamic_cast<Sample*>(selection.second);
  assert(sample);
  
  assert(selection.first.size() == 1); // with one pixel
  Point pos = selection.first.first();
  double bounds[6];
  m_SVA.sva->bounds(bounds);
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
  m_SVA.sva = voi;
  if (m_SVA.sva) 
  {
//     updateTaxonomy();
  } else {
    m_activeValidator = NULL;
  }
  m_validateVesicles->setEnabled(m_SVA.sva != NULL);
}


//-----------------------------------------------------------------------------
void VesicleValidator::updateBounds()
{
  double bounds[6];
  m_SVA.sva->bounds(bounds);
  m_SVA.taxonomy->addProperty("SVA",
		     QString("%1,%2,%3,%4,%5,%6")
		     .arg(bounds[0]).arg(bounds[1])
		     .arg(bounds[2]).arg(bounds[3])
		     .arg(bounds[4]).arg(bounds[5])
  );
  qDebug() << m_SVA.taxonomy->property("SVA").toString();
}

//-----------------------------------------------------------------------------
void VesicleValidator::handleSelection(const ISelectionHandler::MultiSelection& msel)
{
  assert( msel.size() == 1);
  ISelectionHandler::Selelection sel = msel.first();
  EspinaProduct *sample = sel.second;// Only one element is selected
  assert(sample);
  
  assert(sel.first.size() == 1); // with one pixel
  Point pos = sel.first.first();
  
  switch (m_state)
  {
    case DEFINING_SVA:
      defineSVA(pos, sample);
      break;
    case REMOVING_SVA:
      removeSVA(pos, sample);
      break;
    case VALIDATING:
      validateVesicle(sel);
      break;
    case NONE:
    default:
      assert(false);
  };
}


