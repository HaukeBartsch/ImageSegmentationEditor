#ifndef EXPORTSNAPSHOTS_H
#define EXPORTSNAPSHOTS_H

#include <QDialog>

namespace Ui {
  class ExportSnapshots;
}

class ExportSnapshots : public QDialog
{
  Q_OBJECT
  
public:
  explicit ExportSnapshots(QWidget *parent = 0);
  ~ExportSnapshots();

  int getStart();
  int getStop();
  int getStep();
  void setStart( int val );
  void setStop( int val );
  void setStep( int val );
  void setMinMax( int val1, int val2 );
  void updateInfo();
  QWidget *parent;

private slots:
  void on_startSliceSlider_valueChanged(int value);

  void on_stepSliceSlider_valueChanged(int value);

  void on_endSliceSlider_valueChanged(int value);

  void on_buttonBox_accepted();

private:
  Ui::ExportSnapshots *ui;
};

#endif // EXPORTSNAPSHOTS_H
