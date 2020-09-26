#include "stdafx.h"

#include "Import.h"

#include <Standard_WarningsDisable.hxx>
#include <QDir>
#include <QLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QList>
#include <QListView>
#include <QFileDialog>
#include <QApplication>
#include <QWidget>
#include <QStyleFactory>
#include <Standard_WarningsRestore.hxx>

#include <AIS_Shape.hxx>
#include <AIS_InteractiveObject.hxx>

#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <Interface_Static.hxx>
//#include <Interface_TraceFile.hxx>

#include <StlAPI_Writer.hxx>
#include <VrmlAPI_Writer.hxx>

#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_HSequenceOfShape.hxx>

#include <Geom_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>


#include <Standard_ErrorHandler.hxx>
#include <Standard_CString.hxx>

// ---------------------------- ImportDlg -----------------------------------------

class ImportDlg : public QFileDialog
{
public:
	ImportDlg(QWidget* = 0, Qt::WindowFlags flags = 0, bool = true);
	~ImportDlg();
	int                   getMode() const;
	void                  setMode(const int);
	void                  addMode(const int, const QString&);
	void                  clear();

protected:
	void                  showEvent(QShowEvent* event);

private:
	QListView* findListView(const QObjectList&);

private:
	QComboBox* myBox;
	QList<int>            myList;
};

ImportDlg::ImportDlg(QWidget* parent, Qt::WindowFlags flags, bool modal)
	: QFileDialog(parent, flags)
{
	setOption(QFileDialog::DontUseNativeDialog);
	setModal(modal);

	QGridLayout* grid = ::qobject_cast<QGridLayout*>(layout());

	if (grid)
	{
		QVBoxLayout* vbox = new QVBoxLayout;

		QWidget* paramGroup = new QWidget(this);
		paramGroup->setLayout(vbox);

		myBox = new QComboBox(paramGroup);
		vbox->addWidget(myBox);

		int row = grid->rowCount();
		grid->addWidget(paramGroup, row, 1, 1, 3); // make combobox occupy 1 row and 3 columns starting from 1
	}
}

ImportDlg::~ImportDlg()
{
}

int ImportDlg::getMode() const
{
	if (myBox->currentIndex() < 0 || myBox->currentIndex() > (int)myList.count() - 1)
		return -1;
	else
		return myList.at(myBox->currentIndex());
}

void ImportDlg::setMode(const int mode)
{
	int idx = myList.indexOf(mode);
	if (idx >= 0)
		myBox->setCurrentIndex(idx);
}

void ImportDlg::addMode(const int mode, const QString& name)
{
	myBox->show();
	myBox->addItem(name);
	myList.append(mode);
	myBox->updateGeometry();
	updateGeometry();
}

void ImportDlg::clear()
{
	myList.clear();
	myBox->clear();
	myBox->hide();
	myBox->updateGeometry();
	updateGeometry();
}

QListView* ImportDlg::findListView(const QObjectList& childList)
{
	QListView* listView = 0;
	for (int i = 0, n = childList.count(); i < n && !listView; i++)
	{
		listView = qobject_cast<QListView*>(childList.at(i));
		if (!listView && childList.at(i))
		{
			listView = findListView(childList.at(i)->children());
		}
	}
	return listView;
}

void ImportDlg::showEvent(QShowEvent* event)
{
	QFileDialog::showEvent(event);
	QListView* aListView = findListView(children());
	aListView->setViewMode(QListView::ListMode);
}


// ---------------------------- Import -----------------------------------------

Import::Import(QObject* parent)
	: QObject(parent),
	myDlg(0)
{
}

Import::~Import()
{
	if (myDlg)
		delete myDlg;
}

QString Import::info() const
{
	return myInfo;
}

Handle(TopTools_HSequenceOfShape) Import::importModel()
{
	Handle(TopTools_HSequenceOfShape) shapes;

	myInfo = QString();
	int format = Import::FormatNone;
	QString fileName = selectFileName(format);
	if (fileName.isEmpty())
		return shapes;

	if (!QFileInfo(fileName).exists())
	{
		myInfo = QObject::tr("File not found").arg(fileName);
		return shapes;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	shapes = importModel(format, fileName);
	QApplication::restoreOverrideCursor();

	return shapes;    // displayShSequence(ic, shapes);
}

bool Import::displayShSequence(const Handle(AIS_InteractiveContext)& ic,
	const Handle(TopTools_HSequenceOfShape)& shapes)
{
	if (shapes.IsNull() || !shapes->Length())
		return false;

	for (int i = 1; i <= shapes->Length(); i++)
		ic->Display(new AIS_Shape(shapes->Value(i)), false);
	ic->UpdateCurrentViewer();
	return true;
}

Handle(TopTools_HSequenceOfShape) Import::importModel(const int format, const QString& file)
{
	Handle(TopTools_HSequenceOfShape) shapes;
	try {
		switch (format)
		{
			case FormatBREP:
				shapes = importBREP(file);
				break;
			case FormatIGES:
				shapes = importIGES(file);
				break;
			case FormatSTEP:
				shapes = importSTEP(file);
				break;
		}
	}
	catch (Standard_Failure) {
		shapes.Nullify();
	}
	return shapes;
}


Handle(TopTools_HSequenceOfShape) Import::getShapes(const Handle(AIS_InteractiveContext)& ic)
{
	Handle(TopTools_HSequenceOfShape) aSequence;
	Handle(AIS_InteractiveObject) picked;
	for (ic->InitSelected(); ic->MoreSelected(); ic->NextSelected())
	{
		Handle(AIS_InteractiveObject) obj = ic->SelectedInteractive();
		if (obj->IsKind(STANDARD_TYPE(AIS_Shape)))
		{
			TopoDS_Shape shape = Handle(AIS_Shape)::DownCast(obj)->Shape();
			if (aSequence.IsNull())
				aSequence = new TopTools_HSequenceOfShape();
			aSequence->Append(shape);
		}
	}
	return aSequence;
}

/*!
	Selects a file from standard dialog acoording to selection 'filter'
*/
QString Import::selectFileName(int &format)
{
	ImportDlg* theDlg = getDialog();

	int ret = theDlg->exec();

	qApp->processEvents();

	QString file;
	QStringList fileNames;
	if (ret != QDialog::Accepted)
		return file;

	fileNames = theDlg->selectedFiles();
	if (!fileNames.isEmpty())
		file = fileNames[0];

	QString ext = QFileInfo(file).completeSuffix();
	if (!ext.length())
	{
		QString selFilter = theDlg->selectedNameFilter();
		int idx = selFilter.indexOf("(*.");
		if (idx != -1)
		{
			QString tail = selFilter.mid(idx + 3);
			idx = tail.indexOf(" ");
			if (idx == -1)
				idx = tail.indexOf(")");
			QString ext = tail.left(idx);
			if (ext.length())
				file += QString(".") + ext;
		}
	}

	if (ext.compare("igs",Qt::CaseInsensitive) == 0 || ext.compare("iges", Qt::CaseInsensitive) == 0)
		format = Import::FormatIGES;
	else if (ext.compare("stp", Qt::CaseInsensitive) == 0 || ext.compare("step", Qt::CaseInsensitive) == 0)
		format = Import::FormatSTEP;
	else if (ext.compare("brep", Qt::CaseInsensitive) == 0 || ext.compare("rle", Qt::CaseInsensitive) == 0)
		format = Import::FormatBREP;

	return file;
}

ImportDlg* Import::getDialog()
{
	if (!myDlg)
		myDlg = new ImportDlg(0, 0, true);

	QStringList filters({ tr("All CAD files (*.step *.stp *.iges *.igs *.brep *.rle)"),
						  tr("STEP files (*.step *.stp)"),
						  tr("IGES file (*.iges *.igs)"),
						  tr("B-Rep File (*.brep *.rle)"),
						  tr("All Files (*.*)") } );
	myDlg->setNameFilters(filters);

	myDlg->setWindowTitle(QObject::tr("Import Part"));
	((QFileDialog*)myDlg)->setFileMode(QFileDialog::ExistingFile);

	myDlg->clear();

	return myDlg;
}

// ----------------------------- Import functionality -----------------------------

Handle(TopTools_HSequenceOfShape) Import::importBREP(const QString& file)
{
	Handle(TopTools_HSequenceOfShape) aSequence;
	TopoDS_Shape aShape;
	BRep_Builder aBuilder;
	TCollection_AsciiString  aFilePath = file.toUtf8().data();
	Standard_Boolean result = BRepTools::Read(aShape, aFilePath.ToCString(), aBuilder);
	if (result)
	{
		aSequence = new TopTools_HSequenceOfShape();
		aSequence->Append(aShape);
	}
	return aSequence;
}

Handle(TopTools_HSequenceOfShape) Import::importIGES(const QString& file)
{
	Handle(TopTools_HSequenceOfShape) aSequence;
	TCollection_AsciiString  aFilePath = file.toUtf8().data();

	IGESControl_Reader Reader;
	int status = Reader.ReadFile(aFilePath.ToCString());

	if (status == IFSelect_RetDone)
	{
		aSequence = new TopTools_HSequenceOfShape();
		Reader.TransferRoots();
		TopoDS_Shape aShape = Reader.OneShape();
		aSequence->Append(aShape);
	}
	return aSequence;
}

Handle(TopTools_HSequenceOfShape) Import::importSTEP(const QString& file)
{
	Handle(TopTools_HSequenceOfShape) aSequence = new TopTools_HSequenceOfShape;
	TCollection_AsciiString  aFilePath = file.toUtf8().data();
	STEPControl_Reader aReader;
	IFSelect_ReturnStatus status = aReader.ReadFile(aFilePath.ToCString());
	if (status != IFSelect_RetDone)
	{
		return aSequence;
	}

	//Interface_TraceFile::SetDefault();
	bool failsonly = false;
	aReader.PrintCheckLoad(failsonly, IFSelect_ItemsByEntity);

	int nbr = aReader.NbRootsForTransfer();
	aReader.PrintCheckTransfer(failsonly, IFSelect_ItemsByEntity);
	for (Standard_Integer n = 1; n <= nbr; n++)
	{
		aReader.TransferRoot(n);
	}

	int nbs = aReader.NbShapes();
	if (nbs > 0)
	{
		for (int i = 1; i <= nbs; i++)
		{
			TopoDS_Shape shape = aReader.Shape(i);
			aSequence->Append(shape);
		}
	}

	return aSequence;
}


