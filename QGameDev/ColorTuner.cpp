#include <QPalette>
#include <QColorDialog>
#include "ColorTuner.h"
#include "ColorUtils.h"

ColorTuner::ColorTuner(QWidget* parent): QWidget(parent)
{
	m_ui.setupUi(this);
	connect(m_ui.btn, SIGNAL(clicked()), this, SLOT(btn_Click()));
}

ColorTuner::~ColorTuner()
{

}

void ColorTuner::get_color(float& r, float& g, float& b)
{
	r = this->r;
	g = this->g;
	b = this->b;
}

void ColorTuner::set_color(float r, float g, float b)
{
	this->r = r;
	this->g = g;
	this->b = b;
	update();
}

void ColorTuner::update()
{
	QColor color = fromLinear(r, g, b);
	QPalette palette = m_ui.btn->palette();
	palette.setColor(QPalette::ButtonText, color);
	m_ui.btn->setPalette(palette);
}

void ColorTuner::btn_Click()
{
	QPalette palette = m_ui.btn->palette();
	QColor color = QColorDialog::getColor(palette.color(QPalette::ButtonText), this, tr("Select Color"));
	if (!color.isValid()) return;
	float r, g, b;
	toLinear(color, r, g, b);
	set_color(r, g, b);
	emit value_changed();
}
