#include "preferences.h"
#include "ui_preferences.h"
#include "undoredo.h"
#include <QSettings>

Preferences::Preferences(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Preferences)
{
  ui->setupUi(this);

  // set the values in the dialog to the currently active settings
  QSettings settings;
  ui->GrayLowValue->setValue( settings.value("Preferences/grayLowValue", 0.05).toDouble() );
  ui->GrayHighValue->setValue( settings.value("Preferences/grayHighValue", 0.99).toDouble() );
  ui->ColorLowValue->setValue( settings.value("Preferences/colorLowValue", 0.05).toDouble() );
  ui->ColorHighValue->setValue( settings.value("Preferences/colorHighValue", 0.9999).toDouble() );
}

Preferences::~Preferences()
{
  delete ui;
}

void Preferences::on_preferences_undoSteps_valueChanged(int arg1)
{
  QSettings settings;
  settings.setValue("Preferences/undoSteps", arg1);

  // copy these values to the undoRedo function
  UndoRedo::getInstance().maxUndoSteps = arg1;
}

// return number of undo steps
// -1 is a placeholder for infinitely many steps stored
// if the setting does not exist default is 10 steps
int Preferences::undoSteps() {
  QSettings settings;
  return settings.value("Preferences/undoSteps", 10).toInt();
}

// change the brush size
void Preferences::on_preferences_brushSize_editingFinished() {

}

void Preferences::on_preferences_brushSize_valueChanged(int arg1)
{
  QSettings settings;
  settings.setValue("Preferences/brushSize", arg1);
}

// return number of undo steps
// -1 is a placeholder for infinitely many steps stored
// if the setting does not exist default is 10 steps
int Preferences::brushSize() {
  QSettings settings;
  return settings.value("Preferences/brushSize", 1).toInt();
}

QStringList Preferences::getRecentFiles() {
  QSettings settings;
  return settings.value("Preferences/recentFiles", "").toStringList();
}

void Preferences::addToRecentFiles( QString str ) {
  QSettings settings;
  QStringList now = settings.value("Preferences/recentFiles", "").toStringList();
  int idx = now.indexOf(str);
  if (idx != -1) {
    // move to begin
    now.move(idx,0);
  } else {
    now.insert(0, str);
  }
  // make the list not too long
  now.removeDuplicates();
  while(now.length() > getMaxNumberRecentFiles())
    now.removeLast();
  settings.setValue("Preferences/recentFiles", now );
}

QStringList Preferences::getRecentLabels() {
  QSettings settings;
  return settings.value("Preferences/recentLabels", "").toStringList();
}

void Preferences::addToRecentLabels( QString str ) {
  QSettings settings;
  QStringList now = settings.value("Preferences/recentLabels", "").toStringList();
  int idx = now.indexOf(str);
  if (idx != -1) {
    // move to begin
    now.move(idx,0);
  } else {
    now.insert(0, str);
  }
  // make the list not too long
  now.removeDuplicates();
  while(now.length() > getMaxNumberRecentFiles())
    now.removeLast();
  settings.setValue("Preferences/recentLabels", now );
}

bool Preferences::isAutoSaveLabel() {
  QSettings settings;
  return settings.value("Preferences/autoSaveLabel", 1).toInt();
}

// react to auto-save on or off
void Preferences::on_autoSaveLabels_toggled(bool checked)
{
  QSettings settings;
  settings.setValue("Preferences/autoSaveLabel", checked);
}

int Preferences::getMaxNumberRecentFiles() {
  QSettings settings;
  return settings.value("Preferences/maxNumberRecentFiles", 20).toInt();
}


// change number of recent files
void Preferences::on_preferences_NumberRecentFiles_valueChanged(int arg1)
{
  QSettings settings;
  settings.setValue("Preferences/maxNumberRecentFiles", arg1);
}

void Preferences::on_GrayLowValue_valueChanged(double arg1)
{
  QSettings settings;
  settings.setValue("Preferences/grayLowValue", arg1);
}

void Preferences::on_GrayHighValue_valueChanged(double arg1)
{
  QSettings settings;
  settings.setValue("Preferences/grayHighValue", arg1);
}

void Preferences::on_ColorLowValue_valueChanged(double arg1)
{
  QSettings settings;
  settings.setValue("Preferences/colorLowValue", arg1);
}

void Preferences::on_ColorHighValue_valueChanged(double arg1)
{
  QSettings settings;
  settings.setValue("Preferences/colorHighValue", arg1);
}

float Preferences::getGrayLowValue() {
  QSettings settings;
  return settings.value("Preferences/grayLowValue", 0.05).toDouble();
}
float Preferences::getGrayHighValue() {
  QSettings settings;
  return settings.value("Preferences/grayHighValue", 0.99).toDouble();
}
float Preferences::getColorLowValue() {
  QSettings settings;
  return settings.value("Preferences/colorLowValue", 0.05).toDouble();
}
float Preferences::getColorHighValue() {
  QSettings settings;
  return settings.value("Preferences/colorHighValue", 0.9999).toDouble();
}
