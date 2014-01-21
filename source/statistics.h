#ifndef STATISTICS_H
#define STATISTICS_H

#include <QDialog>
#include <QTableWidgetItem>
#include "volume.h"

namespace Ui {
  class Statistics;
}

class Statistics : public QDialog
{
  Q_OBJECT

public:
  explicit Statistics(QWidget *parent = 0);
  ~Statistics();

  void setLabel (ScalarVolume *vol) { this->label = vol; }
  void compute();
  ScalarVolume *label;

private:
  Ui::Statistics *ui;
};

#endif // STATISTICS_H
