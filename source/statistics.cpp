#include <QFileDialog>

#include "statistics.h"
#include "ui_statistics.h"

Statistics::Statistics(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::Statistics)
{
  ui->setupUi(this);
  label = NULL;
}

Statistics::~Statistics()
{
  delete ui;
}

// compute number of voxel for each material
std::vector<long> Statistics::computeHistogram() {
    if (!label)
      return std::vector<long>(); // nothing to do

    // create an array with volume information for each label
    std::vector<long> sizes(label->materialNames.size());
    std::fill(sizes.begin(),sizes.end(),0);

    switch (label->dataType) {
      case MyPrimType::CHAR :
      case MyPrimType::UCHAR : {
          unsigned char *d = label->dataPtr;
          for (long i = 0; i < (long)label->size[0] * label->size[1] * label->size[2]; i++) {
            sizes[d[0]]++;
            d++;
          }
          break;
        }
      case MyPrimType::SHORT : {
          short *d = (short *)label->dataPtr;
          for (long i = 0; i < (long)label->size[0] * label->size[1] * label->size[2]; i++) {
            sizes[d[0]]++;
            d++;
          }
          break;
        }
      case MyPrimType::USHORT : {
          unsigned short *d = (unsigned short *)label->dataPtr;
          for (long i = 0; i < (long)label->size[0] * label->size[1] * label->size[2] ; i++) {
            sizes[d[0]]++;
            d++;
          }
          break;
        }
      case MyPrimType::INT : {
          int *d = (int *)label->dataPtr;
          for (long i = 0; i < (long)label->size[0] * label->size[1] * label->size[2] ; i++) {
            sizes[d[0]]++;
            d++;
          }
          break;
        }
      case MyPrimType::UINT : {
          unsigned int *d = (unsigned int *)label->dataPtr;
          for (long i = 0; i < (long)label->size[0] * label->size[1] * label->size[2]; i++) {
            sizes[d[0]]++;
            d++;
          }
          break;
        }
      case MyPrimType::FLOAT : {
          fprintf(stderr, "Error: label field should never be a floating point field");
          break;
        }
    }
    return sizes;
}

// fill the table with statistics values
void Statistics::compute() {
  std::vector<long> sizes = computeHistogram();

  QColor titleBackground(Qt::lightGray);
  QFont titleFont = ui->tableWidget->font();
  titleFont.setBold(true);

  ui->tableWidget->setRowCount(sizes.size()+2);
  ui->tableWidget->setColumnCount(4);
   // column 0
   ui->tableWidget->setItem(0, 0, new QTableWidgetItem("Name"));
   ui->tableWidget->item(0, 0)->setBackgroundColor(titleBackground);
   ui->tableWidget->item(0, 0)->setToolTip("This column shows the name of a material");
   ui->tableWidget->item(0, 0)->setFont(titleFont);

   ui->tableWidget->setItem(0, 1, new QTableWidgetItem("Value"));
   ui->tableWidget->item(0, 1)->setBackgroundColor(titleBackground);
   ui->tableWidget->item(0, 1)->setToolTip("This column shows the value of each voxel belonging to this material");
   ui->tableWidget->item(0, 1)->setFont(titleFont);

   ui->tableWidget->setItem(0, 2, new QTableWidgetItem("#Voxel"));
   ui->tableWidget->item(0, 2)->setBackgroundColor(titleBackground);
   ui->tableWidget->item(0, 2)->setToolTip("This column shows the number of voxel belonging to that material");
   ui->tableWidget->item(0, 2)->setFont(titleFont);

   ui->tableWidget->setItem(0, 3, new QTableWidgetItem("Volume"));
   ui->tableWidget->item(0, 3)->setBackgroundColor(titleBackground);
   ui->tableWidget->item(0, 3)->setToolTip("This column shows the volume of a material");
   ui->tableWidget->item(0, 3)->setFont(titleFont);

   long sum = 0;
   for (unsigned int i = 0; i < sizes.size(); i++) {
     if (i > 0)
       sum += sizes[i];
     ui->tableWidget->setItem(i+1, 0, new QTableWidgetItem(label->materialNames[i]));
     ui->tableWidget->setItem(i+1, 1, new QTableWidgetItem(QString().sprintf("%d", i)));
     ui->tableWidget->setItem(i+1, 2, new QTableWidgetItem(QString().sprintf("%ld",sizes[i])));
     ui->tableWidget->setItem(i+1, 3, new QTableWidgetItem(QString().sprintf("%f",1.0f*sizes[i]*label->voxelsize[0]*label->voxelsize[1]*label->voxelsize[2])));
   }
   ui->tableWidget->setItem(sizes.size()+1, 2, new QTableWidgetItem(QString().sprintf("%ld",sum)));
   ui->tableWidget->item(sizes.size()+1, 2)->setToolTip("Sum of all non-background label");
   ui->tableWidget->setItem(sizes.size()+1, 3, new QTableWidgetItem(QString().sprintf("%lf",(double)sum*label->voxelsize[0]*label->voxelsize[1]*label->voxelsize[2])));
   ui->tableWidget->item(sizes.size()+1, 3)->setToolTip("Volume of all non-background label");
}

void Statistics::on_okButton_clicked()
{
    fprintf(stderr, "close dialog");
    // close the dialog
    this->hide();
}

// save the statistics into a file
void Statistics::on_saveButton_clicked()
{
    fprintf(stderr, "save statistics into a file");
    if (!label)
        return;

    std::vector<long> sizes = computeHistogram();

    QString filters("CSV files (*.csv);;All files (*.*)");
    QString defaultFilter("CSV files (*.csv)");

    QString fileName = QFileDialog::getSaveFileName(0, "Save Statistics File",
                                                    label->filename,
                                                    filters, &defaultFilter);

    if (fileName.isEmpty())
      return;

    // SaveLabel(fileName);
    FILE *fp;
    if ((fp = fopen(fileName.toLatin1().constData(),"w"))==NULL) {
        fprintf(stderr, "ERROR: could not open file \"%s\" for writing\n", fileName.toLatin1().constData());
        return;
    }

    fprintf(fp, "material name,label value,number of voxel,volume\n");
    long sum = 0;
    for (unsigned int i = 0; i < sizes.size(); i++) {
        if (i > 0)
          sum += sizes[i];
        fprintf(fp,"%s,", label->materialNames[i].toLatin1().constData());
        fprintf(fp, "%d,", i);
        fprintf(fp, "%ld,",sizes[i]);
        fprintf(fp, "%f\n",1.0f*sizes[i]*label->voxelsize[0]*label->voxelsize[1]*label->voxelsize[2]);
    }
    fprintf(fp, ",,%ld,%lf\n",sum,sum*label->voxelsize[0]*label->voxelsize[1]*label->voxelsize[2]);

    fclose(fp);

    // and close the dialog
    this->hide();
}








