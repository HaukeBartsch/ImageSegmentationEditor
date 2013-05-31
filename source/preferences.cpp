#include "preferences.h"
#include "ui_preferences.h"
#include "undoredo.h"
#include <QSettings>

Preferences::Preferences(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Preferences)
{
  ui->setupUi(this);
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
