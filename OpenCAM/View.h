#ifndef VIEW_H
#define VIEW_H

#include <Standard_WarningsDisable.hxx>
#include <QWidget>
#include <QAction>
#include <QList>
#include <Standard_WarningsRestore.hxx>

#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <AIS_Shape.hxx>

#include "Navigation.h"


class TopoDS_Shape;
class QRubberBand;

//class COMMONSAMPLE_EXPORT View: public QWidget
class View: public QWidget
{
    Q_OBJECT
protected:


public:
    enum ViewAction { ViewFitAllId, ViewFitAreaId, ViewZoomId, ViewPanId, ViewGlobalPanId,
                      ViewFrontId, ViewBackId, ViewTopId, ViewBottomId, ViewLeftId, ViewRightId,
                      ViewAxoId, ViewRotationId, ViewResetId, ViewHlrOffId, ViewHlrOnId };
    enum RaytraceAction { ToolRaytracingId, ToolShadowsId, ToolReflectionsId, ToolAntialiasingId };

    View( Handle(AIS_InteractiveContext) theContext, QWidget* parent );

    ~View();

    virtual void                  init();
    bool                          dump( Standard_CString theFile );
    QList<QAction*>*              getViewActions();
    QList<QAction*>*              getRaytraceActions();
    void                          noActiveActions();
    bool                          isShadingMode();

    void                          EnableRaytracing();
    void                          DisableRaytracing();

    void                          SetRaytracedShadows (bool theState);
    void                          SetRaytracedReflections (bool theState);
    void                          SetRaytracedAntialiasing (bool theState);

    bool                          IsRaytracingMode() const { return myIsRaytracing; }
    bool                          IsShadowsEnabled() const { return myIsShadowsEnabled; }
    bool                          IsReflectionsEnabled() const { return myIsReflectionsEnabled; }
    bool                          IsAntialiasingEnabled() const { return myIsAntialiasingEnabled; }

    static QString                GetMessages( int type,TopAbs_ShapeEnum aSubShapeType,
                                               TopAbs_ShapeEnum aShapeType );
    static QString                GetShapeType( TopAbs_ShapeEnum aShapeType );

    Standard_EXPORT static void   OnButtonuseraction( int ExerciceSTEP,
                                                      Handle(AIS_InteractiveContext)& );
    Standard_EXPORT static void   DoSelection( int Id,
                                               Handle(AIS_InteractiveContext)& );
    Standard_EXPORT static void   OnSetSelectionMode( Handle(AIS_InteractiveContext)&,
                                                      Standard_Integer&,
                                                      TopAbs_ShapeEnum& SelectionMode,
                                                      Standard_Boolean& );
    virtual QPaintEngine*         paintEngine() const;
signals:
    void                          selectionChanged();

public slots:
    void                          fitAll();
    void                          fitArea();
    void                          zoom();
    void                          pan();
    void                          globalPan();
    void                          front();
    void                          back();
    void                          top();
    void                          bottom();
    void                          left();
    void                          right();
    void                          axo();
    void                          rotation();
    void                          reset();
    void                          hlrOn();
    void                          hlrOff();
    void                          updateToggled( bool );
    void                          onBackground();
    void                          onEnvironmentMap();
    void                          onRaytraceAction();

    // Navigation events
    void onSelectionChanged();
    void activateCursor(const Navigation::CurrentAction3d);

protected:
    virtual void                  paintEvent( QPaintEvent* );
    virtual void                  resizeEvent( QResizeEvent* );
    virtual void                  mousePressEvent( QMouseEvent* );
    virtual void                  mouseReleaseEvent(QMouseEvent* );
    virtual void                  mouseMoveEvent( QMouseEvent* );
    virtual void                  wheelEvent(QWheelEvent*);
    virtual void                  addItemInPopup( QMenu* );

    Handle(V3d_View)&                     getView();
    Handle(AIS_InteractiveContext)&       getContext();
    void                                  Popup( const int x, const int y );

    //virtual void                          onLButtonDown( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
    //virtual void                          onMButtonDown( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point);
    //virtual void                          onRButtonDown( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point );
    //virtual void                            onLButtonUp( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point );
    //virtual void                            onMButtonUp( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point );
    //virtual void                            onRButtonUp( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point );
    //virtual void                            onMouseMove( Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QPoint point );

private:
    void                          initCursors();
    void                          initViewActions();
    void                          initRaytraceActions();
    void                          DragEvent( const int x, const int y, const int TheState );
    void                          InputEvent( const int x, const int y );
    void                          MoveEvent( const int x, const int y );
    void                          MultiMoveEvent( const int x, const int y );
    void                          MultiDragEvent( const int x, const int y, const int TheState );
    void                          MultiInputEvent( const int x, const int y );
    void                          DrawRectangle( const int MinX, const int MinY,
                                                 const int MaxX, const int MaxY, const bool Draw );

private:
    bool                            myIsRaytracing;
    bool                            myIsShadowsEnabled;
    bool                            myIsReflectionsEnabled;
    bool                            myIsAntialiasingEnabled;

    bool                            myDrawRect;           // set when a rect is used for selection or magnify 
    Handle(V3d_View)                myView;
    Handle(AIS_InteractiveContext)  myContext;
    Standard_Boolean                myHlrModeIsOn;
    QList<QAction*>*                myViewActions;
    QList<QAction*>*                myRaytraceActions;
    QMenu*                          myBackMenu;
    QPointer<Navigation>            navigation;

    void CreateAxis();
};

#endif


