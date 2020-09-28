#if !defined _WIN32
#define QT_CLEAN_NAMESPACE         /* avoid definition of INT32 and INT8 */
#endif

#include "View.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QPainter>
#include <QMenu>
#include <QColorDialog>
#include <QCursor>
#include <QFileInfo>
#include <QFileDialog>
#include <QMouseEvent>
#include <QMdiSubWindow>
#include <QStyleFactory>
#if !defined(_WIN32) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX)) && QT_VERSION < 0x050000
#include <QX11Info>
#endif
#include <Standard_WarningsRestore.hxx>


#include <Graphic3d_GraphicDriver.hxx>
#include <Graphic3d_TextureEnv.hxx>

#include "OcctWindow.h"
#include <Aspect_DisplayConnection.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <Geom_Plane.hxx>
#include <AIS_Plane.hxx>
#include <Geom_Line.hxx>
#include <AIS_Axis.hxx>
#include <Geom_Axis2Placement.hxx>
#include <AIS_Trihedron.hxx>

// Navigation styles
#include "NavigationAlibreClassic.h"


// the key for multi selection :
#define MULTISELECTIONKEY Qt::ShiftModifier

// the key for shortcut ( use to activate dynamic rotation, panning )
#define CASCADESHORTCUTKEY Qt::ControlModifier

static QCursor* defCursor = NULL;
static QCursor* handCursor = NULL;
static QCursor* panCursor = NULL;
static QCursor* globPanCursor = NULL;
static QCursor* zoomCursor = NULL;
static QCursor* rotCursor = NULL;

View::View(Handle(AIS_InteractiveContext) theContext, QWidget* parent)
	: QWidget(parent),
	myIsRaytracing(false),
	myIsShadowsEnabled(true),
	myIsReflectionsEnabled(false),
	myIsAntialiasingEnabled(false),
	myViewActions(0),
	myRaytraceActions(0),
	myBackMenu(NULL)
{
#if !defined(_WIN32) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX)) && QT_VERSION < 0x050000
	XSynchronize(x11Info().display(), true);
#endif
	myContext = theContext;

	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

	myHlrModeIsOn = Standard_False;
	setMouseTracking(true);

	initViewActions();
	initCursors();


	setBackgroundRole(QPalette::NoRole);//NoBackground );
	// set focus policy to threat QContextMenuEvent from keyboard  
	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	init();

	navigation = new NavigationAlibreClassic(this,myContext,myView);
	connect(navigation,SIGNAL(activateCursor(Navigation::CurrentAction3d)), this, SLOT(activateCursor(Navigation::CurrentAction3d)));
	connect(navigation,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));

}

View::~View()
{
	delete myBackMenu;
}

void View::onSelectionChanged()
{
	emit selectionChanged();
}

void View::init()
{
	if (myView.IsNull())
		myView = myContext->CurrentViewer()->CreateView();

	Handle(OcctWindow) hWnd = new OcctWindow(this);
	myView->SetWindow(hWnd);
	if (!hWnd->IsMapped())
	{
		hWnd->Map();
	}
	myView->SetBackgroundColor(Quantity_NOC_BLACK);
	myView->MustBeResized();

	if (myIsRaytracing)
		myView->ChangeRenderingParams().Method = Graphic3d_RM_RAYTRACING;
	myView->SetBgGradientColors(Quantity_NameOfColor::Quantity_NOC_ALICEBLUE, Quantity_NameOfColor::Quantity_NOC_DARKSLATEBLUE, Aspect_GFM_VER);

	CreateAxis();
}

void View::CreateAxis()
{
	{
		//Dynamic
		Handle(Geom_Axis2Placement) myTrihedronAxis = new Geom_Axis2Placement(gp::XOY());
		Handle(AIS_Trihedron) myTrihedron = new AIS_Trihedron(myTrihedronAxis);
		myContext->SetTrihedronSize(200, Standard_False);
		myContext->Display(myTrihedron, AIS_WireFrame, -1, Standard_True);

		// static
		myView->TriedronDisplay(Aspect_TOTP_LEFT_UPPER, Quantity_NOC_BLACK, 0.1, V3d_ZBUFFER);

		//return;
	}


	const Standard_Real planeSize = 100;

	// Planes
	gp_Ax3 xy(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
	Handle(Geom_Plane) plane = new Geom_Plane(xy);
	Handle(AIS_Plane) aShapePrs = new AIS_Plane(plane);
	aShapePrs->SetColor(Quantity_NOC_YELLOW);
	myContext->Display(aShapePrs, AIS_Shaded, -1, true);

	gp_Ax3 yz(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0), gp_Dir(0, 1, 0));
	plane = new Geom_Plane(yz);
	aShapePrs = new AIS_Plane(plane);
	aShapePrs->SetColor(Quantity_NOC_RED);
	myContext->Display(aShapePrs, AIS_Shaded, -1, true);

	gp_Ax3 zx(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0), gp_Dir(0, 0, 1));
	plane = new Geom_Plane(zx);
	aShapePrs = new AIS_Plane(plane);
	aShapePrs->SetColor(Quantity_NOC_PINK);
	myContext->Display(aShapePrs, AIS_Shaded, -1, true);



	//// Axis
	//gp_Ax1 z(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
	//Handle(Geom_Line) axis = new Geom_Line(z);
	//Handle(AIS_Axis) aAxisPrs = new AIS_Axis(axis);
	//aAxisPrs->SetColor(Quantity_NOC_BLUE1);
	//aAxisPrs->SetWidth(1);
	//aAxisPrs->SetTypeOfAxis(AIS_TOAX_ZAxis);
	//myContext->Display(aAxisPrs, AIS_Shaded, 0, true);

	//gp_Ax1 y(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
	//axis = new Geom_Line(y);
	//aAxisPrs = new AIS_Axis(axis);
	//aAxisPrs->SetColor(Quantity_NOC_GREEN);
	//aAxisPrs->SetWidth(1);
	//aAxisPrs->SetTypeOfAxis(AIS_TOAX_YAxis);
	//myContext->Display(aAxisPrs, AIS_Shaded, 0, true);

	//gp_Ax1 x(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
	//axis = new Geom_Line(x);
	//aAxisPrs = new AIS_Axis(axis);
	//aAxisPrs->SetColor(Quantity_NOC_RED);
	//aAxisPrs->SetWidth(1);
	//aAxisPrs->SetTypeOfAxis(AIS_TOAX_XAxis);
	//myContext->Display(aAxisPrs, AIS_Shaded, 0, true);


	//const Standard_Real arrowHeadRadius = 3;
	//const Standard_Real arrowHeadHeight = 10;
	//const Standard_Real arrowHeight = 20;
	//const Standard_Real arrowRadius = 0.5;
	//{
	//    gp_Ax2 z(gp_Pnt(0, 0, arrowHeight), gp_Dir(0, 0, 1));
	//    BRepPrimAPI_MakeCone coneMaker = BRepPrimAPI_MakeCone::BRepPrimAPI_MakeCone(z, arrowHeadRadius, 0, arrowHeadHeight);
	//    TopoDS_Shape shape = coneMaker.Shape();
	//    Handle(AIS_Shape) aShapePrs = new AIS_Shape(shape);
	//    aShapePrs->SetColor(Quantity_NOC_BLUE1);
	//    myContext->Display(aShapePrs, AIS_Shaded, 0, true);

	//    gp_Ax2 z2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
	//    BRepPrimAPI_MakeCylinder cylinderMaker = BRepPrimAPI_MakeCylinder::BRepPrimAPI_MakeCylinder(z2, arrowRadius, arrowHeight);
	//    shape = cylinderMaker.Shape();
	//    aShapePrs = new AIS_Shape(shape);
	//    aShapePrs->SetColor(Quantity_NOC_BLUE1);
	//    myContext->Display(aShapePrs, AIS_Shaded, 0, true);
	//}
	//{
	//    gp_Ax2 y(gp_Pnt(0, arrowHeight, 0), gp_Dir(0, 1, 0));
	//    BRepPrimAPI_MakeCone coneMaker = BRepPrimAPI_MakeCone::BRepPrimAPI_MakeCone(y, arrowHeadRadius, 0, arrowHeadHeight);
	//    TopoDS_Shape shape = coneMaker.Shape();
	//    Handle(AIS_Shape) aShapePrs = new AIS_Shape(shape);
	//    aShapePrs->SetColor(Quantity_NOC_GREEN);
	//    myContext->Display(aShapePrs, AIS_Shaded, 0, true);

	//    gp_Ax2 y2(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
	//    BRepPrimAPI_MakeCylinder cylinderMaker = BRepPrimAPI_MakeCylinder::BRepPrimAPI_MakeCylinder(y2, arrowRadius, arrowHeight);
	//    shape = cylinderMaker.Shape();
	//    aShapePrs = new AIS_Shape(shape);
	//    aShapePrs->SetColor(Quantity_NOC_GREEN);
	//    myContext->Display(aShapePrs, AIS_Shaded, 0, true);
	//}
	//{
	//    gp_Ax2 x(gp_Pnt(arrowHeight, 0, 0), gp_Dir(1, 0, 0));
	//    BRepPrimAPI_MakeCone coneMaker = BRepPrimAPI_MakeCone::BRepPrimAPI_MakeCone(x, arrowHeadRadius, 0, arrowHeadHeight);
	//    TopoDS_Shape shape = coneMaker.Shape();
	//    Handle(AIS_Shape) aShapePrs = new AIS_Shape(shape);
	//    aShapePrs->SetColor(Quantity_NOC_RED);
	//    myContext->Display(aShapePrs, AIS_Shaded, 0, true);

	//    gp_Ax2 x2(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
	//    BRepPrimAPI_MakeCylinder cylinderMaker = BRepPrimAPI_MakeCylinder::BRepPrimAPI_MakeCylinder(x2, arrowRadius, arrowHeight);
	//    shape = cylinderMaker.Shape();
	//    aShapePrs = new AIS_Shape(shape);
	//    aShapePrs->SetColor(Quantity_NOC_RED);
	//    myContext->Display(aShapePrs, AIS_Shaded, 0, true);
	//}
}

void View::paintEvent(QPaintEvent*)
{
	//  QApplication::syncX();
	myView->Redraw();
}

void View::resizeEvent(QResizeEvent*)
{
	//  QApplication::syncX();
	if (!myView.IsNull())
	{
		myView->MustBeResized();
	}
}

void View::fitAll()
{
	myView->FitAll();
	myView->ZFitAll();
	myView->Redraw();
}

void View::fitArea()
{
	//TODO
	//myCurrentMode = CurAction3d_WindowZooming;
}

void View::zoom()
{
	//TODO
	//myCurrentMode = CurAction3d_DynamicZooming;
}

void View::pan()
{
	//TODO
	//myCurrentMode = CurAction3d_DynamicPanning;
}

void View::rotation()
{
	//TODO
	//myCurrentMode = CurAction3d_DynamicRotation;
}

void View::globalPan()
{
	// save the current zoom value
	navigation->SetZoom(myView->Scale() );
	// Do a Global Zoom
	myView->FitAll();
	// Set the mode
	//TODO
	//myCurrentMode = CurAction3d_GlobalPanning;
}

void View::front()
{
	myView->SetProj(V3d_Yneg);
}

void View::back()
{
	myView->SetProj(V3d_Ypos);
}

void View::top()
{
	myView->SetProj(V3d_Zpos);
}

void View::bottom()
{
	myView->SetProj(V3d_Zneg);
}

void View::left()
{
	myView->SetProj(V3d_Xneg);
}

void View::right()
{
	myView->SetProj(V3d_Xpos);
}

void View::axo()
{
	myView->SetProj(V3d_XposYnegZpos);
}

void View::reset()
{
	myView->Reset();
}

void View::hlrOff()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	myHlrModeIsOn = Standard_False;
	myView->SetComputedMode(myHlrModeIsOn);
	myView->Redraw();
	QApplication::restoreOverrideCursor();
}

void View::hlrOn()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	myHlrModeIsOn = Standard_True;
	myView->SetComputedMode(myHlrModeIsOn);
	myView->Redraw();
	QApplication::restoreOverrideCursor();
}

void View::SetRaytracedShadows(bool theState)
{
	myView->ChangeRenderingParams().IsShadowEnabled = theState;

	myIsShadowsEnabled = theState;

	myContext->UpdateCurrentViewer();
}

void View::SetRaytracedReflections(bool theState)
{
	myView->ChangeRenderingParams().IsReflectionEnabled = theState;

	myIsReflectionsEnabled = theState;

	myContext->UpdateCurrentViewer();
}

void View::onRaytraceAction()
{
	QAction* aSentBy = (QAction*)sender();

	if (aSentBy == myRaytraceActions->at(ToolRaytracingId))
	{
		bool aState = myRaytraceActions->at(ToolRaytracingId)->isChecked();

		QApplication::setOverrideCursor(Qt::WaitCursor);
		if (aState)
			EnableRaytracing();
		else
			DisableRaytracing();
		QApplication::restoreOverrideCursor();
	}

	if (aSentBy == myRaytraceActions->at(ToolShadowsId))
	{
		bool aState = myRaytraceActions->at(ToolShadowsId)->isChecked();
		SetRaytracedShadows(aState);
	}

	if (aSentBy == myRaytraceActions->at(ToolReflectionsId))
	{
		bool aState = myRaytraceActions->at(ToolReflectionsId)->isChecked();
		SetRaytracedReflections(aState);
	}

	if (aSentBy == myRaytraceActions->at(ToolAntialiasingId))
	{
		bool aState = myRaytraceActions->at(ToolAntialiasingId)->isChecked();
		SetRaytracedAntialiasing(aState);
	}
}

void View::SetRaytracedAntialiasing(bool theState)
{
	myView->ChangeRenderingParams().IsAntialiasingEnabled = theState;

	myIsAntialiasingEnabled = theState;

	myContext->UpdateCurrentViewer();
}

void View::EnableRaytracing()
{
	if (!myIsRaytracing)
		myView->ChangeRenderingParams().Method = Graphic3d_RM_RAYTRACING;

	myIsRaytracing = true;

	myContext->UpdateCurrentViewer();
}

void View::DisableRaytracing()
{
	if (myIsRaytracing)
		myView->ChangeRenderingParams().Method = Graphic3d_RM_RASTERIZATION;

	myIsRaytracing = false;

	myContext->UpdateCurrentViewer();
}

void View::updateToggled(bool isOn)
{
	QAction* sentBy = (QAction*)sender();

	if (!isOn)
		return;

	for (int i = ViewFitAllId; i < ViewHlrOffId; i++)
	{
		QAction* anAction = myViewActions->at(i);

		if ((anAction == myViewActions->at(ViewFitAreaId)) ||
			(anAction == myViewActions->at(ViewZoomId)) ||
			(anAction == myViewActions->at(ViewPanId)) ||
			(anAction == myViewActions->at(ViewGlobalPanId)) ||
			(anAction == myViewActions->at(ViewRotationId)))
		{
			if (anAction && (anAction != sentBy))
			{
				anAction->setCheckable(true);
				anAction->setChecked(false);
			}
			else
			{
				if (sentBy == myViewActions->at(ViewFitAreaId))
					setCursor(*handCursor);
				else if (sentBy == myViewActions->at(ViewZoomId))
					setCursor(*zoomCursor);
				else if (sentBy == myViewActions->at(ViewPanId))
					setCursor(*panCursor);
				else if (sentBy == myViewActions->at(ViewGlobalPanId))
					setCursor(*globPanCursor);
				else if (sentBy == myViewActions->at(ViewRotationId))
					setCursor(*rotCursor);
				else
					setCursor(*defCursor);

				sentBy->setCheckable(false);
			}
		}
	}
}

void View::initCursors()
{
	if (!defCursor)
		defCursor = new QCursor(Qt::ArrowCursor);
	if (!handCursor)
		handCursor = new QCursor(Qt::PointingHandCursor);
	if (!panCursor)
		panCursor = new QCursor(Qt::SizeAllCursor);
	if (!globPanCursor)
		globPanCursor = new QCursor(Qt::CrossCursor);
	//if ( !zoomCursor )
	//  zoomCursor = new QCursor( QPixmap( ApplicationCommonWindow::getResourceDir() + QString( "/" ) + QObject::tr( "ICON_CURSOR_ZOOM" ) ) );
	//if ( !rotCursor )
	//  rotCursor = new QCursor( QPixmap( ApplicationCommonWindow::getResourceDir() + QString( "/" ) + QObject::tr( "ICON_CURSOR_ROTATE" ) ) );
}

QList<QAction*>* View::getViewActions()
{
	initViewActions();
	return myViewActions;
}

QList<QAction*>* View::getRaytraceActions()
{
	initRaytraceActions();
	return myRaytraceActions;
}

/*!
  Get paint engine for the OpenGL viewer. [ virtual public ]
*/
QPaintEngine* View::paintEngine() const
{
	return 0;
}

void View::initViewActions()
{
	if (myViewActions)
		return;

	myViewActions = new QList<QAction*>();
	QAction* a;

	a = new QAction( QPixmap( ":/ICON/VIEW_FITALL" ), QObject::tr("&Fit All"), this );
	a->setToolTip( QObject::tr("Zoom Fit All") );
	a->setStatusTip( QObject::tr("Zoom Fit All") );
	connect( a, SIGNAL( triggered() ) , this, SLOT( fitAll() ) );
	myViewActions->insert(ViewFitAllId, a);

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_FITAREA") ), QObject::tr("MNU_FITAREA"), this );
	//a->setToolTip( QObject::tr("TBR_FITAREA") );
	//a->setStatusTip( QObject::tr("TBR_FITAREA") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( fitArea() ) );

	//a->setCheckable( true );
	//connect( a, SIGNAL( toggled( bool ) ) , this, SLOT( updateToggled( bool ) ) );
	//myViewActions->insert( ViewFitAreaId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_ZOOM") ), QObject::tr("MNU_ZOOM"), this );
	//a->setToolTip( QObject::tr("TBR_ZOOM") );
	//a->setStatusTip( QObject::tr("TBR_ZOOM") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( zoom() ) );

	//a->setCheckable( true );
	//connect( a, SIGNAL( toggled(bool) ) , this, SLOT( updateToggled(bool) ) );
	//myViewActions->insert( ViewZoomId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_PAN") ), QObject::tr("MNU_PAN"), this );
	//a->setToolTip( QObject::tr("TBR_PAN") );
	//a->setStatusTip( QObject::tr("TBR_PAN") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( pan() ) );

	//a->setCheckable( true );
	//connect( a, SIGNAL( toggled(bool) ) , this, SLOT( updateToggled(bool) ) );
	//myViewActions->insert( ViewPanId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_GLOBALPAN") ), QObject::tr("MNU_GLOBALPAN"), this );
	//a->setToolTip( QObject::tr("TBR_GLOBALPAN") );
	//a->setStatusTip( QObject::tr("TBR_GLOBALPAN") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( globalPan() ) );

	//a->setCheckable( true );
	//connect( a, SIGNAL( toggled(bool) ) , this, SLOT( updateToggled(bool) ) );
	//myViewActions->insert( ViewGlobalPanId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_FRONT") ), QObject::tr("MNU_FRONT"), this );
	//a->setToolTip( QObject::tr("TBR_FRONT") );
	//a->setStatusTip( QObject::tr("TBR_FRONT") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( front() ) );
	//myViewActions->insert( ViewFrontId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_BACK") ), QObject::tr("MNU_BACK"), this );
	//a->setToolTip( QObject::tr("TBR_BACK") );
	//a->setStatusTip( QObject::tr("TBR_BACK") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( back() ) );
	//myViewActions->insert(ViewBackId, a);

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_TOP") ), QObject::tr("MNU_TOP"), this );
	//a->setToolTip( QObject::tr("TBR_TOP") );
	//a->setStatusTip( QObject::tr("TBR_TOP") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( top() ) );
	//myViewActions->insert( ViewTopId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_BOTTOM") ), QObject::tr("MNU_BOTTOM"), this );
	//a->setToolTip( QObject::tr("TBR_BOTTOM") );
	//a->setStatusTip( QObject::tr("TBR_BOTTOM") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( bottom() ) );
	//myViewActions->insert( ViewBottomId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_LEFT") ), QObject::tr("MNU_LEFT"), this );
	//a->setToolTip( QObject::tr("TBR_LEFT") );
	//a->setStatusTip( QObject::tr("TBR_LEFT") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( left() ) );
	//myViewActions->insert( ViewLeftId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_RIGHT") ), QObject::tr("MNU_RIGHT"), this );
	//a->setToolTip( QObject::tr("TBR_RIGHT") );
	//a->setStatusTip( QObject::tr("TBR_RIGHT") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( right() ) );
	//myViewActions->insert( ViewRightId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_AXO") ), QObject::tr("MNU_AXO"), this );
	//a->setToolTip( QObject::tr("TBR_AXO") );
	//a->setStatusTip( QObject::tr("TBR_AXO") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( axo() ) );
	//myViewActions->insert( ViewAxoId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_ROTATION") ), QObject::tr("MNU_ROTATION"), this );
	//a->setToolTip( QObject::tr("TBR_ROTATION") );
	//a->setStatusTip( QObject::tr("TBR_ROTATION") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( rotation() ) );
	//a->setCheckable( true );
	//connect( a, SIGNAL( toggled(bool) ) , this, SLOT( updateToggled(bool) ) );
	//myViewActions->insert( ViewRotationId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_RESET") ), QObject::tr("MNU_RESET"), this );
	//a->setToolTip( QObject::tr("TBR_RESET") );
	//a->setStatusTip( QObject::tr("TBR_RESET") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( reset() ) );
	//myViewActions->insert( ViewResetId, a );

	//QActionGroup* ag = new QActionGroup( this );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_HLROFF") ), QObject::tr("MNU_HLROFF"), this );
	//a->setToolTip( QObject::tr("TBR_HLROFF") );
	//a->setStatusTip( QObject::tr("TBR_HLROFF") );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( hlrOff() ) );
	//a->setCheckable( true );
	//ag->addAction(a);
	//myViewActions->insert(ViewHlrOffId, a);

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_VIEW_HLRON") ), QObject::tr("MNU_HLRON"), this );
	//a->setToolTip( QObject::tr("TBR_HLRON") );
	//a->setStatusTip( QObject::tr("TBR_HLRON") );
	//connect( a, SIGNAL( triggered() ) ,this, SLOT( hlrOn() ) );
	//
	//a->setCheckable( true );
	//ag->addAction(a);
	//myViewActions->insert( ViewHlrOnId, a );
}

void View::initRaytraceActions()
{
	if (myRaytraceActions)
		return;

	myRaytraceActions = new QList<QAction*>();
	//QString dir = ApplicationCommonWindow::getResourceDir() + QString( "/" );
	//QAction* a;

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_RAYTRACING") ), QObject::tr("MNU_TOOL_RAYTRACING"), this );
	//a->setToolTip( QObject::tr("TBR_TOOL_RAYTRACING") );
	//a->setStatusTip( QObject::tr("TBR_TOOL_RAYTRACING") );
	//a->setCheckable( true );
	//a->setChecked( false );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( onRaytraceAction() ) );
	//myRaytraceActions->insert( ToolRaytracingId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_SHADOWS") ), QObject::tr("MNU_TOOL_SHADOWS"), this );
	//a->setToolTip( QObject::tr("TBR_TOOL_SHADOWS") );
	//a->setStatusTip( QObject::tr("TBR_TOOL_SHADOWS") );
	//a->setCheckable( true );
	//a->setChecked( true );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( onRaytraceAction() ) );
	//myRaytraceActions->insert( ToolShadowsId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_REFLECTIONS") ), QObject::tr("MNU_TOOL_REFLECTIONS"), this );
	//a->setToolTip( QObject::tr("TBR_TOOL_REFLECTIONS") );
	//a->setStatusTip( QObject::tr("TBR_TOOL_REFLECTIONS") );
	//a->setCheckable( true );
	//a->setChecked( false );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( onRaytraceAction() ) );
	//myRaytraceActions->insert( ToolReflectionsId, a );

	//a = new QAction( QPixmap( dir+QObject::tr("ICON_TOOL_ANTIALIASING") ), QObject::tr("MNU_TOOL_ANTIALIASING"), this );
	//a->setToolTip( QObject::tr("TBR_TOOL_ANTIALIASING") );
	//a->setStatusTip( QObject::tr("TBR_TOOL_ANTIALIASING") );
	//a->setCheckable( true );
	//a->setChecked( false );
	//connect( a, SIGNAL( triggered() ) , this, SLOT( onRaytraceAction() ) );
	//myRaytraceActions->insert( ToolAntialiasingId, a );
}

void View::mousePressEvent(QMouseEvent* e)
{
	navigation->mousePressEvent(e);
}

void View::mouseReleaseEvent(QMouseEvent* e)
{
	navigation->mouseReleaseEvent(e);
}

void View::mouseMoveEvent(QMouseEvent* e)
{
	navigation->mouseMoveEvent(e);
}

void View::wheelEvent(QWheelEvent* e)
{
	navigation->wheelEvent(e);
}

void View::activateCursor(const Navigation::CurrentAction3d mode)
{
	//switch( mode )
	//{
	//  case CurAction3d_DynamicPanning:
	//    setCursor( *panCursor );
	//    break;
	//  case CurAction3d_DynamicZooming:
	//    setCursor( *zoomCursor );
	//    break;
	//  case CurAction3d_DynamicRotation:
	//    setCursor( *rotCursor );
	//    break;
	//  case CurAction3d_GlobalPanning:
	//    setCursor( *globPanCursor );
	//    break;
	//  case CurAction3d_WindowZooming:
	//    setCursor( *handCursor );
	//    break;
	//  case CurAction3d_Nothing:
	//  default:
	//    setCursor( *defCursor );
	//    break;
	//}
}


void View::Popup(const int /*x*/, const int /*y*/)
{
	//ApplicationCommonWindow* stApp = ApplicationCommonWindow::getApplication();
	//QMdiArea* ws = ApplicationCommonWindow::getWorkspace();
	//QMdiSubWindow* w = ws->activeSubWindow();
	//if ( myContext->NbSelected() )
	//{
	//  QList<QAction*>* aList = stApp->getToolActions();
	//  QMenu* myToolMenu = new QMenu( 0 );
	//  myToolMenu->addAction( aList->at( ApplicationCommonWindow::ToolWireframeId ) );
	//  myToolMenu->addAction( aList->at( ApplicationCommonWindow::ToolShadingId ) );
	//  myToolMenu->addAction( aList->at( ApplicationCommonWindow::ToolColorId ) );
	//      
	//  QMenu* myMaterMenu = new QMenu( myToolMenu );

	//  QList<QAction*>* aMeterActions = ApplicationCommonWindow::getApplication()->getMaterialActions();
	//      
	//  QString dir = ApplicationCommonWindow::getResourceDir() + QString( "/" );
	//  myMaterMenu = myToolMenu->addMenu( QPixmap( dir+QObject::tr("ICON_TOOL_MATER")), QObject::tr("MNU_MATER") );
	//  for ( int i = 0; i < aMeterActions->size(); i++ )
	//    myMaterMenu->addAction( aMeterActions->at( i ) );
	//     
	//  myToolMenu->addAction( aList->at( ApplicationCommonWindow::ToolTransparencyId ) );
	//  myToolMenu->addAction( aList->at( ApplicationCommonWindow::ToolDeleteId ) );
	//  addItemInPopup(myToolMenu);
	//  myToolMenu->exec( QCursor::pos() );
	//  delete myToolMenu;
	//}
	//else
	//{
	//  if (!myBackMenu)
	//  {
	//    myBackMenu = new QMenu( 0 );

	//    QAction* a = new QAction( QObject::tr("MNU_CH_BACK"), this );
	//    a->setToolTip( QObject::tr("TBR_CH_BACK") );
	//    connect( a, SIGNAL( triggered() ), this, SLOT( onBackground() ) );
	//    myBackMenu->addAction( a );  
	//    addItemInPopup(myBackMenu);

	//    a = new QAction( QObject::tr("MNU_CH_ENV_MAP"), this );
	//    a->setToolTip( QObject::tr("TBR_CH_ENV_MAP") );
	//    connect( a, SIGNAL( triggered() ), this, SLOT( onEnvironmentMap() ) );
	//    a->setCheckable( true );
	//    a->setChecked( false );
	//    myBackMenu->addAction( a );  
	//    addItemInPopup(myBackMenu);
	//  }

	//  myBackMenu->exec( QCursor::pos() );
	//}
	//if ( w )
	//  w->setFocus();
}

void View::addItemInPopup(QMenu* /*theMenu*/)
{
}

void View::noActiveActions()
{
	for ( int i = ViewFitAllId; i < ViewHlrOffId ; i++ )
	{
	    QAction* anAction = myViewActions->at( i );
	    if( ( anAction == myViewActions->at( ViewFitAreaId ) ) ||
	        ( anAction == myViewActions->at( ViewZoomId ) ) ||
	        ( anAction == myViewActions->at( ViewPanId ) ) ||
	        ( anAction == myViewActions->at( ViewGlobalPanId ) ) ||
	        ( anAction == myViewActions->at( ViewRotationId ) ) )
	    {
	        setCursor( *defCursor );
	        anAction->setCheckable( true );
	        anAction->setChecked( false );
	    }
	}
}

void View::onBackground()
{
	QColor aColor;
	Standard_Real R1;
	Standard_Real G1;
	Standard_Real B1;
	myView->BackgroundColor(Quantity_TOC_RGB, R1, G1, B1);
	aColor.setRgb((Standard_Integer)(R1 * 255), (Standard_Integer)(G1 * 255), (Standard_Integer)(B1 * 255));

	QColor aRetColor = QColorDialog::getColor(aColor);

	if (aRetColor.isValid())
	{
		R1 = aRetColor.red() / 255.;
		G1 = aRetColor.green() / 255.;
		B1 = aRetColor.blue() / 255.;
		myView->SetBackgroundColor(Quantity_TOC_RGB, R1, G1, B1);
	}
	myView->Redraw();
}

void View::onEnvironmentMap()
{
	if (myBackMenu->actions().at(1)->isChecked())
	{
		QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
			tr("All Image Files (*.bmp *.gif *.jpg *.jpeg *.png *.tga)"));

		const TCollection_AsciiString anUtf8Path(fileName.toUtf8().data());

		Handle(Graphic3d_TextureEnv) aTexture = new Graphic3d_TextureEnv(anUtf8Path);

		myView->SetTextureEnv(aTexture);
	}
	else
	{
		myView->SetTextureEnv(Handle(Graphic3d_TextureEnv)());
	}

	myView->Redraw();
}

bool View::dump(Standard_CString theFile)
{
	return myView->Dump(theFile);
}

Handle(V3d_View)& View::getView()
{
	return myView;
}

Handle(AIS_InteractiveContext)& View::getContext()
{
	return myContext;
}

//View::CurrentAction3d View::getCurrentMode()
//{
//	return myCurrentMode;
//}



