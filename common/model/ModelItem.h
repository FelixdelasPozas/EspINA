/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef MODELITEM_H
#define MODELITEM_H

#include <QModelIndex>

#include <QSharedPointer>
#include <QStringList>

class Representation;
class ModelItemExtension;
class RelationshipGraph;
class VertexProperty;

template<class T>
QString arg3(const T val[3])
{
  return QString("%1,%2,%3").arg(val[0]).arg(val[1]).arg(val[2]);
}
template<class T>
QString arg6(const T val[6])
{
  return QString("%1,%2,%3,%4,%5,%6").arg(val[0]).arg(val[1]).arg(val[2]).arg(val[3]).arg(val[4]).arg(val[5]);
}


/// Base class for every item in EspinaModel
class ModelItem
{
public:
  struct Relation
  {
    ModelItem *ancestor;
    ModelItem *succesor;
    QString relation;
  };

  typedef QList<Relation> RelationList;
  
  class Arguments : public QMap<QString, QString>
  {
  public:
    explicit Arguments();
    explicit Arguments(const QMap<QString, QString>& args);
    explicit Arguments(const QString args);
    virtual ~Arguments(){}

    virtual QString serialize() const;
    virtual QString hash() const;
  protected:
    inline QString argument(const QString name, const QString value) const
    {
      return QString("%1=%2;").arg(name).arg(value);
    }
  };

  typedef QList<ModelItem *> Vector;
  enum ItemType
  { TAXONOMY
  , SAMPLE
  , CHANNEL
  , SEGMENTATION
  , FILTER};

  enum RelationType
  { IN
  , OUT
  , INOUT
  };

  ModelItem() : m_vertex(0), m_relations(NULL) {}
  virtual ~ModelItem(){}

  virtual QString  id() const = 0;
  virtual QVariant data(int role) const = 0;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole +1) {return false;}
  virtual QString  serialize() const {return QString("none");}
  virtual ItemType type() const = 0;
  
  Vector relatedItems(RelationType rel, const QString filter = "");
  RelationList relations(const QString filter = "");

  virtual QStringList availableInformations() const;
  virtual QStringList availableRepresentations() const;
  virtual QVariant information(QString name) const;
  virtual Representation *representation(QString name) const;

protected:
  void addExtension(ModelItemExtension *ext);

  size_t             m_vertex;
  RelationshipGraph *m_relations;

  QMap<QString, ModelItemExtension *> m_extensions;
  QMap<QString, ModelItemExtension *> m_pendingExtensions;
  QList<ModelItemExtension *>         m_insertionOrderedExtensions;
  QMap<QString, Representation *>     m_representations;
  QMap<QString, ModelItemExtension *> m_informations;

  friend class RelationshipGraph;
};

typedef QSharedPointer<ModelItem> ModelItemPtr;

ModelItem *indexPtr(const QModelIndex &index);

#endif //MODELITEM_H
