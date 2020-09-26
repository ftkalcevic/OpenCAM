#include "Navigation.h"

#include <QStyleFactory>


Navigation::Navigation(QWidget *parent, Handle(AIS_InteractiveContext) theContext, Handle(V3d_View) theView) :
	  QObject(parent),
	  myContext(theContext),
	  myView(theView)
{
	myCurrentMode = CurAction3d_Nothing;
	myXmin = 0;
	myYmin = 0;
	myXmax = 0;
	myYmax = 0;
	myCurZoom = 0;
	myParent = parent;
}

Navigation::~Navigation()
{
}

void Navigation::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		onLButtonDown(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() == Qt::MidButton)
		onMButtonDown(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() == Qt::RightButton)
		onRButtonDown(e->buttons(), e->modifiers(), e->pos());
}

void Navigation::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
		onLButtonUp(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() == Qt::MidButton)
		onMButtonUp(e->buttons(), e->modifiers(), e->pos());
	else if (e->button() == Qt::RightButton)
		onRButtonUp(e->buttons(), e->modifiers(), e->pos());
}

void Navigation::mouseMoveEvent(QMouseEvent* e)
{
	onMouseMove(e->buttons(), e->modifiers(), e->pos());
}

void Navigation::wheelEvent(QWheelEvent* e)
{
	//const double ANGLE_SCALE = 10;
	//e->angleDelta();
	//e->modifiers();
	//e->pos();
	//myView->StartZoomAtPoint(e->pos().x(), e->pos().y());
	//myView->ZoomAtPoint(0, 0, e->angleDelta().x() / ANGLE_SCALE, e->angleDelta().y() / ANGLE_SCALE);
}


void Navigation::onLButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	//  save the current mouse coordinate in min
	myXmin = point.x();
	myYmin = point.y();
	myXmax = point.x();
	myYmax = point.y();

	if (modifiers & Qt::ControlModifier)
	{
		myCurrentMode = CurAction3d_DynamicZooming;
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

void Navigation::onMButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	if (modifiers & Qt::ControlModifier)
		myCurrentMode = CurAction3d_DynamicPanning;
	emit activateCursor(myCurrentMode);
}

void Navigation::onRButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	if (modifiers & Qt::ControlModifier)
	{
		//TODO
		//if (myHlrModeIsOn)
		//{
		//	myView->SetComputedMode(Standard_False);
		//}
		myCurrentMode = CurAction3d_DynamicRotation;
		myView->StartRotation(point.x(), point.y());
	}
	else
	{
		//TODO
		//Popup(point.x(), point.y());
	}
	emit activateCursor(myCurrentMode);
}


void Navigation::onLButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
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

void Navigation::onMButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	myCurrentMode = CurAction3d_Nothing;
	emit activateCursor(myCurrentMode);
}

void Navigation::onRButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
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

void Navigation::onMouseMove(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point)
{
	if (buttons & Qt::LeftButton || buttons & Qt::RightButton || buttons & Qt::MidButton)
	{
		switch (myCurrentMode)
		{
			case CurAction3d_Nothing:
				myXmax = point.x();
				myYmax = point.y();
				DrawRectangle(myXmin, myYmin, myXmax, myYmax, Standard_False);
				if (modifiers & Qt::ShiftModifier)
					MultiDragEvent(myXmax, myYmax, 0);
				else
					DragEvent(myXmax, myYmax, 0);
				DrawRectangle(myXmin, myYmin, myXmax, myYmax, Standard_True);
				break;
			case CurAction3d_DynamicZooming:
				myView->Zoom(myXmax, myYmax, point.x(), point.y());
				myXmax = point.x();
				myYmax = point.y();
				break;
			case CurAction3d_WindowZooming:
				myXmax = point.x();
				myYmax = point.y();
				DrawRectangle(myXmin, myYmin, myXmax, myYmax, Standard_False);
				DrawRectangle(myXmin, myYmin, myXmax, myYmax, Standard_True);
				break;
			case CurAction3d_DynamicPanning:
				myView->Pan(point.x() - myXmax, myYmax - point.y());
				myXmax = point.x();
				myYmax = point.y();
				break;
			case CurAction3d_GlobalPanning:
				break;
			case CurAction3d_DynamicRotation:
				myView->Rotation(point.x(), point.y());
				myView->Redraw();
				break;
			default:
				throw Standard_Failure("incompatible Current Mode");
				break;
		}
	}
	else
	{
		myXmax = point.x();
		myYmax = point.y();
		if (modifiers & Qt::ShiftModifier)
			MultiMoveEvent(point.x(), point.y());
		else
			MoveEvent(point.x(), point.y());
	}
}

void Navigation::DragEvent(const int x, const int y, const int TheState)
{
	// TheState == -1  button down
	// TheState ==  0  move
	// TheState ==  1  button up

	static Standard_Integer theButtonDownX = 0;
	static Standard_Integer theButtonDownY = 0;

	if (TheState == -1)
	{
		theButtonDownX = x;
		theButtonDownY = y;
	}

	if (TheState == 1)
	{
		myContext->Select(theButtonDownX, theButtonDownY, x, y, myView, Standard_True);
		emit selectionChanged();
	}
}

void Navigation::InputEvent(const int /*x*/, const int /*y*/)
{
	myContext->Select(Standard_True);
	emit selectionChanged();
}

void Navigation::MoveEvent(const int x, const int y)
{
	myContext->MoveTo(x, y, myView, Standard_True);
}

void Navigation::MultiMoveEvent(const int x, const int y)
{
	myContext->MoveTo(x, y, myView, Standard_True);
}

void Navigation::MultiDragEvent(const int x, const int y, const int TheState)
{
	static Standard_Integer theButtonDownX = 0;
	static Standard_Integer theButtonDownY = 0;

	if (TheState == -1)
	{
		theButtonDownX = x;
		theButtonDownY = y;
	}
	if (TheState == 0)
	{
		myContext->ShiftSelect(theButtonDownX, theButtonDownY, x, y, myView, Standard_True);
		emit selectionChanged();
	}
}

void Navigation::MultiInputEvent(const int /*x*/, const int /*y*/)
{
	myContext->ShiftSelect(Standard_True);
	emit selectionChanged();
}


void Navigation::DrawRectangle(const int MinX, const int MinY,
	const int MaxX, const int MaxY, const bool Draw)
{
	static Standard_Integer StoredMinX, StoredMaxX, StoredMinY, StoredMaxY;
	static Standard_Boolean m_IsVisible;

	StoredMinX = (MinX < MaxX) ? MinX : MaxX;
	StoredMinY = (MinY < MaxY) ? MinY : MaxY;
	StoredMaxX = (MinX > MaxX) ? MinX : MaxX;
	StoredMaxY = (MinY > MaxY) ? MinY : MaxY;

	QRect aRect;
	aRect.setRect(StoredMinX, StoredMinY, abs(StoredMaxX - StoredMinX), abs(StoredMaxY - StoredMinY));

	if (!myRectBand)
	{
		myRectBand = new QRubberBand(QRubberBand::Rectangle, myParent);
		myRectBand->setStyle(QStyleFactory::create("windows"));
		myRectBand->setGeometry(aRect);
		myRectBand->show();

		/*QPalette palette;
		palette.setColor(myRectBand->foregroundRole(), Qt::white);
		myRectBand->setPalette(palette);*/
	}

	if (m_IsVisible && !Draw) // move or up  : erase at the old position
	{
		myRectBand->hide();
		delete myRectBand;
		myRectBand = 0;
		m_IsVisible = false;
	}

	if (Draw) // move : draw
	{
		//aRect.setRect( StoredMinX, StoredMinY, abs(StoredMaxX-StoredMinX), abs(StoredMaxY-StoredMinY));
		m_IsVisible = true;
		myRectBand->setGeometry(aRect);
		//myRectBand->show();
	}
}


void Navigation::noActiveActions()
{
	// TODO
}


void Navigation::SetWaitCursor()
{
	emit activateCursor(CurAction3d_Wait);
}

void Navigation::RestoreCursor()
{
	emit activateCursor(myCurrentMode);
}
