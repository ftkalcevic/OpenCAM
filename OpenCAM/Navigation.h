#pragma once

#include <QObject>
#include <QMouseEvent>
#include <QPointer>
#include <QRubberBand>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>


// for elastic bean selection
const int ValZWMin = 1;


class Navigation : public QObject
{
	Q_OBJECT

public:
	Navigation(QWidget *parent, Handle(AIS_InteractiveContext) theContext, Handle(V3d_View) myView );
	~Navigation();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);

	enum CurrentAction3d {
		CurAction3d_Nothing, 
		CurAction3d_DynamicZooming,
		CurAction3d_WindowZooming, 
		CurAction3d_DynamicPanning,
		CurAction3d_GlobalPanning, 
		CurAction3d_DynamicRotation,
		CurAction3d_Wait
	};

	void SetZoom(Standard_Real z) { myCurZoom = z; }

signals:
	void activateCursor(Navigation::CurrentAction3d mode);
	void selectionChanged();

protected:
	Handle(V3d_View) myView;
	Handle(AIS_InteractiveContext)  myContext;

	void onLButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onMButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onRButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onLButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onMButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onRButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onMouseMove(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void DragEvent(const int x, const int y, const int TheState);
	void InputEvent(const int /*x*/, const int /*y*/);
	void MoveEvent(const int x, const int y);
	void MultiMoveEvent(const int x, const int y);
	void MultiDragEvent(const int x, const int y, const int TheState);
	void MultiInputEvent(const int /*x*/, const int /*y*/);
	void DrawRectangle(const int MinX, const int MinY, const int MaxX, const int MaxY, const bool Draw);
	void noActiveActions();
	void SetWaitCursor();
	void RestoreCursor();

	Standard_Integer                myXmin;
	Standard_Integer                myYmin;
	Standard_Integer                myXmax;
	Standard_Integer                myYmax;
	Standard_Real                   myCurZoom;
	CurrentAction3d myCurrentMode;
	QPointer<QRubberBand> myRectBand; //!< selection rectangle rubber band
	QWidget* myParent;
};
