#pragma once

#include "Navigation.h"

class AlibreClassicNavigation : public Navigation
{
	Q_OBJECT

public:
	AlibreClassicNavigation(QWidget* parent, Handle(AIS_InteractiveContext) theContext, Handle(V3d_View) theView);
	~AlibreClassicNavigation();

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);
	void onLButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onMButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onRButtonDown(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onLButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onMButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
	void onRButtonUp(Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);

};
