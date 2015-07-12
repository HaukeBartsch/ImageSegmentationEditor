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
  std::vector<long> computeHistogram();
  ScalarVolume *label;

private slots:

  void on_okButton_clicked();

  void on_saveButton_clicked();

private:
  Ui::Statistics *ui;
};

#endif // STATISTICS_H
