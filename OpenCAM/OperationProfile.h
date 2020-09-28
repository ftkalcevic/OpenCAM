#pragma once

#include "Operation.h"
#include "ui_OperationProfile.h"
#include "Shape.h"

class OperationProfile : public Operation
{
	Q_OBJECT

public:
	OperationProfile(QWidget *parent, Handle(AIS_InteractiveContext) context);
	virtual ~OperationProfile();

	virtual void onShapeSelectionChanged(const QList< QSharedPointer<Shape> > &selections);

public slots:
	void onOkClicked();
	void onCancelClicked();
	void onGeometrySelectGeometry();
	void onGeometryListSelectionChanged();

private:
	Ui::OperationProfile ui;
};
