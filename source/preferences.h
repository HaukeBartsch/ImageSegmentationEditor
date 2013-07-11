#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QString>
#include <QStringList>
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
  int brushSize();  /// return the current brush size
  bool isAutoSaveLabel(); /// return if we auto save images
  QStringList getRecentFiles();
  void addToRecentFiles( QString str );
  QStringList getRecentLabels();
  void addToRecentLabels( QString str );
  int getMaxNumberRecentFiles();
  float getGrayLowValue();
  float getGrayHighValue();
  float getColorLowValue();
  float getColorHighValue();

private slots:
  void on_preferences_undoSteps_valueChanged(int arg1);

  void on_preferences_brushSize_editingFinished();

  void on_preferences_brushSize_valueChanged(int arg1);

  void on_autoSaveLabels_toggled(bool checked);

  void on_preferences_NumberRecentFiles_valueChanged(int arg1);

  void on_GrayLowValue_valueChanged(double arg1);

  void on_GrayHighValue_valueChanged(double arg1);

  void on_ColorLowValue_valueChanged(double arg1);

  void on_ColorHighValue_valueChanged(double arg1);

private:
  Ui::Preferences *ui;
};

#endif // PREFERENCES_H
