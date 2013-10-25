/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "ClassificationXML.h"
#include <Core/Analysis/Classification.h>

#include <QXmlStreamReader>

using namespace EspINA;
using namespace EspINA::IO;

//-----------------------------------------------------------------------------
STATUS ClassificationXML::load(const QFileInfo&   file,
                               ClassificationSPtr classification,
                               ErrorHandlerPtr    handler)
{
  //QFile xmlFile(file);
//   if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
//   {
    return STATUS::IO_ERROR;
//   }
// 
//   QXmlStreamReader xmlStream(&xmlFile);
// 
//   TaxonomySPtr tax = readXML(xmlStream);
// 
//   xmlFile.close();

  return STATUS::SUCCESS;
}

//-----------------------------------------------------------------------------
void dumpCategoryXML(CategorySPtr category, QXmlStreamWriter& stream) 
{
  stream.writeStartElement("category");
  stream.writeAttribute("name", category->name());
  stream.writeAttribute("color", category->color().name());

  foreach(QString prop, category->properties()){
    stream.writeAttribute(prop, category->property(prop).toString());
  }

  foreach(CategorySPtr subCategory, category->subCategories()){
    dumpCategoryXML(subCategory, stream);
  }

  stream.writeEndElement();
}


//-----------------------------------------------------------------------------
STATUS dumpClassificationXML(ClassificationSPtr classification, QXmlStreamWriter& stream) {

  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  stream.writeStartElement("classification");
  stream.writeAttribute("name", classification->name());

  foreach(CategorySPtr category, classification->categories())
  {
    dumpCategoryXML(category, stream);
  }

  stream.writeEndElement();
  stream.writeEndDocument();

  return STATUS::SUCCESS;
}

//-----------------------------------------------------------------------------
STATUS ClassificationXML::save(ClassificationSPtr classification,
                               const QFileInfo&   file,
                               ErrorHandlerPtr    handler)
{
  QFile xmlFile(file.absoluteFilePath());
  if (!xmlFile.open(QIODevice::WriteOnly))
  {
    return STATUS::IO_ERROR;
  }

  QXmlStreamWriter stream(&xmlFile);

  return dumpClassificationXML(classification, stream);
}


//-----------------------------------------------------------------------------
QByteArray ClassificationXML::dump(const ClassificationSPtr classification,
                                   ErrorHandlerPtr handler)
{
  QByteArray serialization;
  QXmlStreamWriter stream(&serialization);

  dumpClassificationXML(classification, stream);

  return serialization;
}

//-----------------------------------------------------------------------------
ClassificationSPtr ClassificationXML::parse(const QByteArray& serialization,
                                            ErrorHandlerPtr handler)
{
  return ClassificationSPtr();
}
