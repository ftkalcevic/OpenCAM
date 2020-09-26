#include "PartView.h"

PartView::PartView(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

PartView::~PartView()
{
}
