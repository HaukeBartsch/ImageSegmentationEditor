#include "exportsnapshots.h"
#include "ui_exportsnapshots.h"
#include <qfile>
#include "mainwindow.h"
#include <math.h>

ExportSnapshots::ExportSnapshots(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ExportSnapshots)
{
  ui->setupUi(this);
  this->parent = parent;
}

ExportSnapshots::~ExportSnapshots()
{
  delete ui;
}

int ExportSnapshots::getStart() {
  return ui->startSliceSlider->value();
}
int ExportSnapshots::getStop() {
  return ui->endSliceSlider->value();
}
int ExportSnapshots::getStep() {
  return ui->stepSliceSlider->value();
}
void ExportSnapshots::setStart( int value ) {
  ui->startSliceSlider->setValue( value );
}
void ExportSnapshots::setStop( int value ) {
  ui->endSliceSlider->setValue( value );
}
void ExportSnapshots::setStep( int value ) {
  ui->stepSliceSlider->setValue( value );
}
void ExportSnapshots::setMinMax( int val1, int val2 ) {
  ui->stepSliceSlider->setMinimum(val1+1);
  ui->stepSliceSlider->setMaximum(val2);
  ui->endSliceSlider->setMinimum(val1);
  ui->endSliceSlider->setMaximum(val2);
  ui->startSliceSlider->setMinimum(val1);
  ui->startSliceSlider->setMaximum(val2);
}

void ExportSnapshots::updateInfo() {
  int numImages = abs(floor((getStop()-getStart())/getStep()));
  QString str = QString("start: %1, end: %2, steps: %3 (%4 images)").arg(getStart()).arg(getStop()).arg(getStep()).arg(numImages);
  ui->infoSnapshots->setText(str);
}

void ExportSnapshots::on_startSliceSlider_valueChanged(int value)
{
  updateInfo();
  // move the image

  if (parent) {
    ((MainWindow *)parent)->setMainWindowPos(value);
    ((MainWindow *)parent)->update();
  }
}

void ExportSnapshots::on_stepSliceSlider_valueChanged(int value)
{
  updateInfo();
}

void ExportSnapshots::on_endSliceSlider_valueChanged(int value)
{
  updateInfo();
  if (parent) {
    ((MainWindow *)parent)->setMainWindowPos(value);
    ((MainWindow *)parent)->update();
  }
}

// start saving the images
void ExportSnapshots::on_buttonBox_accepted()
{
  QString format = "png";
  QString initialPath = QDir::currentPath() + tr("/snapshots.") + format;

  QString fileName = QFileDialog::getSaveFileName(parent, tr("Save As"),
                              initialPath,
                              tr("%1 Files (*.%2);;All Files (*)")
                              .arg(format.toUpper())
                              .arg(format));
  // move the image from start to end
  if (getStart() < getStop()) {
    for (int i = getStart(); i < getStop(); i+=getStep()) {
      ((MainWindow *)parent)->setMainWindowPos(i);
      ((MainWindow *)parent)->update();
      QString fn = QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName() +
          QString("%1").arg(i, 4, 10, QChar('0')) + QString(".png");
      ((MainWindow *)parent)->snapshot( fn );
    }
  } else if (getStart() > getStop()) {
    for (int i = getStart(); i > getStop()-1; i-=getStep()) {
      ((MainWindow *)parent)->setMainWindowPos(i);
      ((MainWindow *)parent)->update();
      QString fn = QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName() +
          QString("%1").arg(i, 4, 10, QChar('0')) + QString(".png");
      ((MainWindow *)parent)->snapshot( fn );
    }
  }
  this->hide();
}
