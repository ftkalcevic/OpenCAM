#pragma once

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QMap>

#include <TopoDS.hxx>

class Shape: public QObject
{
	Q_OBJECT

public:
	Shape(const TopoDS_Shape& tshape, QString name);
	virtual ~Shape();

	const TopoDS_Shape tshape;
	QString name;

	void Add(QSharedPointer<Shape> shape);

	QList< QSharedPointer<Shape> > children;
	static QMap< QString, QSharedPointer<Shape> > nameMap;

	QSharedPointer<Shape> FindShape(const TopoDS_Shape& aSelShape);

};

