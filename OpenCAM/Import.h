#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <Standard_WarningsDisable.hxx>
#include <QObject>
#include <Standard_WarningsRestore.hxx>

#include <AIS_InteractiveContext.hxx>
#include <TopTools_HSequenceOfShape.hxx>

class ImportDlg;

class Import : public QObject
{
	Q_OBJECT

public:
	enum { FormatNone, FormatBREP, FormatIGES, FormatSTEP };

	Import(QObject*);
	~Import();

	Handle(TopTools_HSequenceOfShape) importModel();
	Handle(TopTools_HSequenceOfShape) importModel(const int, const QString&);

	QString									info() const;

protected:
	virtual bool                              displayShSequence(const Handle(AIS_InteractiveContext)&,
																const Handle(TopTools_HSequenceOfShape)&);
	QString                                   selectFileName(int &);

private:
	ImportDlg* getDialog();
	Handle(TopTools_HSequenceOfShape)         getShapes(const Handle(AIS_InteractiveContext)&);

	Handle(TopTools_HSequenceOfShape)         importBREP(const QString&);
	Handle(TopTools_HSequenceOfShape)         importIGES(const QString&);
	Handle(TopTools_HSequenceOfShape)         importSTEP(const QString&);


protected:
	ImportDlg* myDlg;
	QString  myInfo;
};

#endif

