#include "Shape.h"
#include <QDebug>

QMap< QString, QSharedPointer<Shape> > Shape::nameMap;

Shape::Shape(const TopoDS_Shape& tshape, const QString name) :
	tshape(tshape),
	name(name)
{
	qDebug() << "New tshape " << (uint64_t)(tshape.TShape().get());
}

Shape::~Shape()
{
	//if ( nameMap.contains(name) )
	//	nameMap.remove(name);
}

void Shape::Add(QSharedPointer<Shape> shape) 
{
	children.append(shape);
	nameMap.insert(shape->name, shape);
}

QSharedPointer<Shape> Shape::FindShape(const TopoDS_Shape& aSelShape)
{
	for (auto it = children.cbegin(); it != children.cend(); it++)
	{
		QSharedPointer<Shape> shape = *it;

		if (shape->tshape.IsEqual(aSelShape))
			return shape;

		QSharedPointer<Shape> match = shape->FindShape(aSelShape);
		if (!match.isNull())
			return match;
	}
	return QSharedPointer<Shape>();
}

