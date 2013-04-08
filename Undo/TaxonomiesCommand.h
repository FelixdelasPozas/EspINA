/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2013  <copyright holder> <email>
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


#ifndef TAXONOMIESCOMMAND_H
#define TAXONOMIESCOMMAND_H

#include <QUndoCommand>

#include <Core/EspinaTypes.h>
#include <Core/Model/Taxonomy.h>

#include <QMap>

namespace EspINA
{
  class EspinaModel;

  class AddTaxonomyCommand
  : public QUndoCommand
  {
  public:
    explicit AddTaxonomyCommand(TaxonomySPtr  taxonomy,
                                EspinaModel  *model,
                                QUndoCommand *parent = NULL);
    virtual void redo();
    virtual void undo();

  private:
    void swapTaxonomy();

  private:
    EspinaModel *m_model;

    TaxonomySPtr m_prevTaxonomy;
  };

  // Add new taxonomical element to parentTaxonomy
  class AddTaxonomyElement
  : public QUndoCommand
  {
  public:
    explicit AddTaxonomyElement(TaxonomyElementSPtr parentTaxonomy,
                                TaxonomyElementSPtr element,
                                EspinaModel        *model,
                                QUndoCommand       *parent = NULL);

    explicit AddTaxonomyElement(TaxonomyElementPtr parentTaxonomy,
                                const QString     &name,
                                EspinaModel       *model,
                                QColor            color,
                                QUndoCommand      *parent = NULL);
    virtual ~AddTaxonomyElement();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    QString m_name;
    QColor  m_color;
    TaxonomyElementSPtr m_taxonomy;
    TaxonomyElementSPtr m_parentTaxonomy;
  };


  // Change taxonomical element's parent
  class MoveTaxonomiesCommand
  : public QUndoCommand
  {
  public:
    explicit MoveTaxonomiesCommand(TaxonomyElementList taxonomies,
                                   TaxonomyElementPtr  parentTaxonomy,
                                   EspinaModel         *model,
                                   QUndoCommand        *parent = NULL);
    virtual ~MoveTaxonomiesCommand();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    TaxonomyElementSPtr m_parentTaxonomy;
    QMap<TaxonomyElementSPtr, TaxonomyElementSPtr> m_oldTaxonomyParents;
  };


  // Remove Taxonomical Element from model
  class RemoveTaxonomyElementCommand
  : public QUndoCommand
  {
  public:
    explicit RemoveTaxonomyElementCommand(TaxonomyElementPtr taxonomy,
                                          EspinaModel       *model,
                                          QUndoCommand      *parent=NULL);
    virtual ~RemoveTaxonomyElementCommand();

    virtual void redo();
    virtual void undo();

  private:
    EspinaModel *m_model;

    TaxonomyElementSPtr m_taxonomy;
    TaxonomyElementSPtr m_parent;
  };

} // namespace EspINA

#endif // TAXONOMIESCOMMAND_H
