#pragma once

#include <QDialog>
#include <QSharedPointer>
#include <QList>

#include <AIS_InteractiveContext.hxx>

#include "Shape.h"

class Operation : public QDialog
{
	Q_OBJECT

protected:
	Operation(QWidget* parent, Handle(AIS_InteractiveContext) context);

	Handle(AIS_InteractiveContext) context;

public:
	virtual ~Operation();

	virtual void onShapeSelectionChanged(const QList< QSharedPointer<Shape> >& selections) = 0;

	void ChangeModal(bool modal);
};
