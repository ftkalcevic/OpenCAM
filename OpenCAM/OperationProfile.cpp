#include "OperationProfile.h"
#include <QMessageBox>
#include <QDebug>

#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <AIS_Shape.hxx>

OperationProfile::OperationProfile(QWidget *parent, Handle(AIS_InteractiveContext) context )
	: Operation(parent, context)
{
	ui.setupUi(this);
}

OperationProfile::~OperationProfile()
{
}


void OperationProfile::onOkClicked()
{
	accept();
}

void OperationProfile::onCancelClicked()
{
	reject();
}


void OperationProfile::onGeometrySelectGeometry()
{
	ChangeModal(!isModal());
}


void OperationProfile::onShapeSelectionChanged(const QList< QSharedPointer<Shape> >& selections)
{
	ui.lstGeometryGeometry->clear();
	foreach(QSharedPointer<Shape> s, selections)
	{
		QListWidgetItem* item = new QListWidgetItem(s->name);
		item->setData(Qt::UserRole,QVariant::fromValue(s));
		ui.lstGeometryGeometry->addItem(item);
	}
}

void OperationProfile::onGeometryListSelectionChanged()
{
	//bool autoHiglight = context->AutomaticHilight();
	//for (int i = 0; i < ui.lstGeometryGeometry->count(); i++)
	//{
	//	QListWidgetItem* item = ui.lstGeometryGeometry->item(i);
	//	QSharedPointer<Shape> shape = item->data(Qt::UserRole).value< QSharedPointer<Shape> >();
	//	Handle(AIS_Shape) ais_shape(new AIS_Shape(shape->tshape));
	//	Handle(Prs3d_Drawer) drawer = context->HighlightStyle();
	//	drawer->SetColor(Quantity_NameOfColor::Quantity_NOC_HOTPINK);
	//	context->HilightWithColor(ais_shape, drawer, true);
	//	
	//}
}
