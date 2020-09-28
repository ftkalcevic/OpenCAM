#pragma once

#include <QFileDialog>
#include <QListView>
#include <QList>
#include <QComboBox>

class ImportDlg : public QFileDialog
{
	Q_OBJECT

public:
	ImportDlg(QWidget* parent, Qt::WindowFlags flags, bool modal);
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
