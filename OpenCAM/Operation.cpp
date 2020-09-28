#include "Operation.h"

Operation::Operation(QWidget* parent, Handle(AIS_InteractiveContext) ctx)
	: QDialog(parent), 
	  context(ctx)
{
}

Operation::~Operation()
{
}


void Operation::ChangeModal(bool modal)
{
	QPoint location = pos();
	hide();
	setModal(modal);
	move(location);
	show();
}