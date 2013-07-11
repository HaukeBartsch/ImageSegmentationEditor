#include "setbrightnesscontrast.h"
#include "ui_setbrightnesscontrast.h"
#include "mainwindow.h"
#include <math.h>

SetBrightnessContrast::SetBrightnessContrast(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SetBrightnessContrast)
{
  ui->setupUi(this);
  this->parent = parent;
}

void SetBrightnessContrast::setLowIntensity( float a ) {
  ui->lowIntensity->setValue( a );
}
void SetBrightnessContrast::setHighIntensity( float a ) {
  ui->highIntensity->setValue( a );
}
float SetBrightnessContrast::getLowIntensity() {
  return ui->lowIntensity->value();
}
float SetBrightnessContrast::getHighIntensity() {
  return ui->highIntensity->value();
}
void SetBrightnessContrast::setRange(float a, float b) {
  ui->lowIntensity->setMinimum(a - fabs(b-a));
  ui->lowIntensity->setMaximum(b + fabs(b-a));
  ui->highIntensity->setMinimum(a - fabs(b-a));
  ui->highIntensity->setMaximum(b + fabs(b-a));
}

SetBrightnessContrast::~SetBrightnessContrast()
{
  delete ui;
}

// do something with the entered value
void SetBrightnessContrast::on_buttonBox_accepted()
{
  float a = getLowIntensity();
  float b = getHighIntensity();
  if (parent) {
    ((MainWindow *)parent)->setCurrentWindowLevel(a,b);
    ((MainWindow *)parent)->update();
  }
}

void SetBrightnessContrast::on_lowIntensity_valueChanged(double arg1)
{
  on_buttonBox_accepted(); // update display every time value changes
}

void SetBrightnessContrast::on_highIntensity_valueChanged(double arg1)
{
  on_buttonBox_accepted(); // update display every time value changes
}
