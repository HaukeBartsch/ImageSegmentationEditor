#ifndef SETBRIGHTNESSCONTRAST_H
#define SETBRIGHTNESSCONTRAST_H

#include <QDialog>

namespace Ui {
  class SetBrightnessContrast;
}

class SetBrightnessContrast : public QDialog
{
  Q_OBJECT
  
public:
  explicit SetBrightnessContrast(QWidget *parent);
  ~SetBrightnessContrast();
  void setLowIntensity( float a );
  void setHighIntensity( float a );
  float getLowIntensity();
  float getHighIntensity();
  void setRange(float a, float b);

private slots:
  void on_buttonBox_accepted();

  void on_lowIntensity_valueChanged(double arg1);

  void on_highIntensity_valueChanged(double arg1);

private:
  Ui::SetBrightnessContrast *ui;
  QWidget *parent;
};

#endif // SETBRIGHTNESSCONTRAST_H
