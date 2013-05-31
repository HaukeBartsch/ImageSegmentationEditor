#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
  class Preferences;
}

class Preferences : public QDialog
{
  Q_OBJECT
  
public:
  explicit Preferences(QWidget *parent = 0);
  ~Preferences();
  
  int undoSteps();  /// return number of undo steps

private slots:
  void on_preferences_undoSteps_valueChanged(int arg1);

private:
  Ui::Preferences *ui;
};

#endif // PREFERENCES_H
