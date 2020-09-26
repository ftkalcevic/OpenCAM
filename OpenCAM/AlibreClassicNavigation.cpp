#include "AlibreClassicNavigation.h"

#include <QDebug>

AlibreClassicNavigation::AlibreClassicNavigation(QWidget* parent, Handle(AIS_InteractiveContext) theContext, Handle(V3d_View) theView)
	: Navigation(parent, theContext, theView)
{
}

AlibreClassicNavigation::~AlibreClassicNavigation()
{
}


// Zoom - Scroll Wheel
// Pan - middle button
// Rotate - left + right button




void AlibreClassicNavigation::mousePressEvent(QMouseEvent* e)
{
	qDebug() << e->buttons();
	if (e->button() & Qt::LeftButton)
		onLButtonDown(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() & Qt::MidButton)
		onMButtonDown(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() & Qt::RightButton)
		onRButtonDown(e->buttons(), e->modifiers(), e->pos());
}

void AlibreClassicNavigation::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() & Qt::LeftButton)
		onLButtonUp(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() & Qt::MidButton)
		onMButtonUp(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() & Qt::RightButton)
		onRButtonUp(e->buttons(), e->modifiers(), e->pos());
}

void AlibreClassicNavigation::mouseMoveEvent(QMouseEvent* e)
{
	onMouseMove(e->buttons(), e->modifiers(), e->pos());
}

void AlibreClassicNavigation::wheelEvent(QWheelEvent* e)
{
	const double ANGLE_SCALE = 10;
	myView->StartZoomAtPoint(e->pos().x(), e->pos().y());
	myView->ZoomAtPoint(0, 0, e->angleDelta().x() / ANGLE_SCALE, e->angleDelta().y() / ANGLE_SCALE);
}


void AlibreClassicNavigation::onLButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	//  save the current mouse coordinate in min
	myXmin = point.x();
	myYmin = point.y();
	myXmax = point.x();
	myYmax = point.y();

	if ((buttons & (Qt::RightButton| Qt::LeftButton)) == (Qt::RightButton | Qt::LeftButton) && modifiers == 0 )
	{
		myCurrentMode = CurAction3d_DynamicRotation;
	}
	else
	{
		switch (myCurrentMode)
		{
			case CurAction3d_Nothing:
				if (modifiers & Qt::ShiftModifier)
					MultiDragEvent(myXmax, myYmax, -1);
				else
					DragEvent(myXmax, myYmax, -1);
				break;
			case CurAction3d_DynamicZooming:
				break;
			case CurAction3d_WindowZooming:
				break;
			case CurAction3d_DynamicPanning:
				break;
			case CurAction3d_GlobalPanning:
				break;
			case CurAction3d_DynamicRotation:
				//TODO
				//if (myHlrModeIsOn)
				//{
				//	myView->SetComputedMode(Standard_False);
				//}
				myView->StartRotation(point.x(), point.y());
				break;
			default:
				throw Standard_Failure("incompatible Current Mode");
				break;
		}
	}
	emit activateCursor(myCurrentMode);
}

void AlibreClassicNavigation::onMButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	myCurrentMode = CurAction3d_DynamicPanning;
	emit activateCursor(myCurrentMode);
}

void AlibreClassicNavigation::onRButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	if ((buttons & (Qt::RightButton | Qt::LeftButton)) == (Qt::RightButton | Qt::LeftButton) && modifiers == 0)
	{
		myCurrentMode = CurAction3d_DynamicRotation;
	}
	else
	{
		//TODO
		//Popup(point.x(), point.y());
	}
	emit activateCursor(myCurrentMode);
}


void AlibreClassicNavigation::onLButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	switch (myCurrentMode)
	{
		case CurAction3d_Nothing:
			if (point.x() == myXmin && point.y() == myYmin)
			{
				// no offset between down and up --> selectEvent
				myXmax = point.x();
				myYmax = point.y();
				if (modifiers & Qt::ShiftModifier)
					MultiInputEvent(point.x(), point.y());
				else
					InputEvent(point.x(), point.y());
			}
			else
			{
				DrawRectangle(myXmin, myYmin, myXmax, myYmax, Standard_False);
				myXmax = point.x();
				myYmax = point.y();
				if (modifiers & Qt::ShiftModifier)
					MultiDragEvent(point.x(), point.y(), 1);
				else
					DragEvent(point.x(), point.y(), 1);
			}
			break;
		case CurAction3d_DynamicZooming:
			myCurrentMode = CurAction3d_Nothing;
			noActiveActions();
			break;
		case CurAction3d_WindowZooming:
			DrawRectangle(myXmin, myYmin, myXmax, myYmax, Standard_False);//,LongDash);
			myXmax = point.x();
			myYmax = point.y();
			if ((abs(myXmin - myXmax) > ValZWMin) ||
				(abs(myYmin - myYmax) > ValZWMin))
				myView->WindowFitAll(myXmin, myYmin, myXmax, myYmax);
			myCurrentMode = CurAction3d_Nothing;
			noActiveActions();
			break;
		case CurAction3d_DynamicPanning:
			myCurrentMode = CurAction3d_Nothing;
			noActiveActions();
			break;
		case CurAction3d_GlobalPanning:
			myView->Place(point.x(), point.y(), myCurZoom);
			myCurrentMode = CurAction3d_Nothing;
			noActiveActions();
			break;
		case CurAction3d_DynamicRotation:
			myCurrentMode = CurAction3d_Nothing;
			noActiveActions();
			break;
		default:
			throw Standard_Failure(" incompatible Current Mode ");
			break;
	}
	emit activateCursor(myCurrentMode);
	//ApplicationCommonWindow::getApplication()->onSelectionChanged();
}

void AlibreClassicNavigation::onMButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	myCurrentMode = CurAction3d_Nothing;
	emit activateCursor(myCurrentMode);
}

void AlibreClassicNavigation::onRButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	if (myCurrentMode == CurAction3d_Nothing)
	{
		// TODO
		//Popup(point.x(), point.y());
	}
	else
	{
		SetWaitCursor();
		// reset tyhe good Degenerated mode according to the strored one
		//   --> dynamic rotation may have change it
		//TODO
		//if (myHlrModeIsOn)
		//{
		//	myView->SetComputedMode(myHlrModeIsOn);
		//	myView->Redraw();
		//}
		RestoreCursor();
		myCurrentMode = CurAction3d_Nothing;
	}
	emit activateCursor(myCurrentMode);
}

