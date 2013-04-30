#include <QPainter>
#include <QShortcut>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "readmgz.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setWindowTitle(tr("Image Segmentation Editor"));
    ui->setupUi(this);

    ui->Image1->setBackgroundRole(QPalette::Base);
    ui->Image1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->Image1->setScaledContents(true);
    ui->Image1->setMouseTracking(true);

    ui->Image2->setBackgroundRole(QPalette::Base);
    ui->Image2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->Image2->setScaledContents(true);
    ui->Image2->setMouseTracking(true);

    ui->Image3->setBackgroundRole(QPalette::Base);
    ui->Image3->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->Image3->setScaledContents(true);
    ui->Image3->setMouseTracking(true);

    ui->scrollArea_4->setBackgroundRole(QPalette::Dark);
    ui->scrollArea_2->setBackgroundRole(QPalette::Dark);
    ui->scrollArea_3->setBackgroundRole(QPalette::Dark);

    ui->treeWidget->setColumnCount(3);
    QStringList headerText;
    headerText << tr("Material") << tr("Index") << tr("Color");
    ui->treeWidget->setHeaderLabels(headerText);

    // setupDefaultMaterials();

    createActions();

    vol1 = NULL;
    lab1 = NULL;
    windowLevel[0] = 0;
    windowLevel[1] = 2400;
    windowLevelOverlay[0] = 0;
    windowLevelOverlay[1] = 255;
    mouseIsDown = false;

    toolWindowLevel = false;
    toolSegmentation = false;

    scaleFactor1  = 1.8;
    scaleFactor23 = 1.4;

    qsrand(1234);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::myKeyPressEvent(QObject *object, QKeyEvent *keyEvent) {
  if (object != ui->Image1 && object != ui->Image2 && object != ui->Image3 ) {
    QWidget::keyPressEvent(keyEvent);
    return;
  }
  //fprintf(stderr, "MainWindow event received at: %d %d\n", keyEvent->pos().x(), mouseEvent->pos().y());
  if (object == ui->Image1) {
    if (keyEvent->matches(QKeySequence::MoveToPreviousLine) ||
        keyEvent->key() == Qt::Key_Up) {
      slicePosition[2]++;
      update();
    } else if (keyEvent->key() == Qt::Key_Down) {
      slicePosition[2]--;
      update();
    } else if (keyEvent->key() == Qt::Key_F) {
      FillHighlight(0);
    }
  } else if (object == ui->Image2) {
    if (keyEvent->matches(QKeySequence::MoveToPreviousLine) ||
        keyEvent->key() == Qt::Key_Up) {
      slicePosition[1]++;
      update();
    } else if (keyEvent->key() == Qt::Key_Down) {
      slicePosition[1]--;
      update();
    } else if (keyEvent->key() == Qt::Key_F) {
      FillHighlight(1);
    }
  } else if (object == ui->Image3) {
    if (keyEvent->matches(QKeySequence::MoveToPreviousLine) ||
        keyEvent->key() == Qt::Key_Up) {
      slicePosition[0]++;
      update();
    } else if (keyEvent->key() == Qt::Key_Down) {
      slicePosition[0]--;
      update();
    } else if (keyEvent->key() == Qt::Key_F) {
      FillHighlight(2);
    }
  }
  QWidget::keyPressEvent(keyEvent);
  // extract the intensity at the mouse location and show as text

}

void MainWindow::myMouseReleaseEvent ( QObject *object, QMouseEvent * e ) {
  //fprintf(stderr, "Mouse was released at: %d %d", e->pos().x(), e->pos().y() );
  mouseIsDown = false;
}

void MainWindow::myMousePressEvent ( QObject *object, QMouseEvent * e ) {
  if (object != ui->Image1 && object != ui->Image2 && object != ui->Image3 ) {
      return;
  }
  //fprintf(stderr, "Mouse was pressed at: %d %d", e->pos().x(), e->pos().y() );
  mousePressLocation[0] = e->pos().x();
  mousePressLocation[1] = e->pos().y();
  windowLevelBefore[0]  = windowLevel[0];
  windowLevelBefore[1]  = windowLevel[1];
  mouseIsDown = true;
}

void MainWindow::mouseEvent(QObject *object, QMouseEvent *e) {
  if (object != ui->Image1 && object != ui->Image2 && object != ui->Image3 ) {
    return;
  }
  //fprintf(stderr, "MainWindow event received at: %d %d\n", mouseEvent->pos().x(), mouseEvent->pos().y());
  if (slicePosition.size() == 3)
    ui->label->setText(QString("x: %1 y: %2 z: %3 (%4,%5)")
                       .arg(slicePosition[0])
        .arg(slicePosition[1])
        .arg(slicePosition[2])
        .arg( floor( e->pos().x() / scaleFactor1 + 0.5) )
        .arg( floor( e->pos().y() / scaleFactor23 + 0.5) ));

  // extract the intensity at the mouse location and show as text
  if (mouseIsDown && toolWindowLevel) {
    float distx = (mousePressLocation[0] - e->pos().x());
    float disty = (mousePressLocation[1] - e->pos().y());
    //windowLevel[0] = windowLevelBefore[0] + disty;
    //windowLevel[1] = windowLevelBefore[1] + disty;
    float mid  = windowLevelBefore[0] + (windowLevelBefore[1]-windowLevelBefore[0])/2.0;
    float cont = (windowLevelBefore[1]-windowLevelBefore[0]);
    windowLevel[0] = disty + mid-(cont+distx)/2.0;
    windowLevel[1] = disty + mid+(cont+distx)/2.0;
    if (windowLevel[1]<windowLevel[0]) {
      float tmp = windowLevel[0];
      windowLevel[0] = windowLevel[1];
      windowLevel[1] = tmp;
    }
    fprintf(stderr, "window level change: %f %f", windowLevel[0], windowLevel[1]);

    update();
  } else if (mouseIsDown && toolSegmentation) {
    // can be several tools create a brush first
    if (lab1 && object == ui->Image1) {
      int posx = floor( e->pos().x() / scaleFactor1 + 0.5);
      int posy = floor( e->pos().y() / scaleFactor1 + 0.5);

      bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;
      int radius = 1;
      for (int i = posx-radius; i <= posx+radius; i++) {
        for (int j = posy-radius; j <= posy+radius; j++) {
          hbuffer[slicePosition[2]*(lab1->size[0]*lab1->size[1]) + j*lab1->size[0] + i] = !isCtrl;
        }
      }
      update();
    }

  }
}

void MainWindow::myMouseWheelEvent (QObject *object, QWheelEvent * e) {
  if (object != ui->Image1 && object != ui->Image2 && object != ui->Image3 )
    return;

  int numDegrees = e->delta() / 8;
  int numSteps = numDegrees / 15;

  if (object == ui->Image1) {
    if (e->orientation() == Qt::Vertical) {
      slicePosition[2] += numSteps;
      update();
    }
    e->accept();
  } else if (object == ui->Image2) {
    if (e->orientation() == Qt::Vertical) {
      slicePosition[1] += numSteps;
      update();
    }
    e->accept();
  } else if (object == ui->Image3) {
    if (e->orientation() == Qt::Vertical) {
      slicePosition[0] += numSteps;
      update();
    }
    e->accept();
  }
}

void MainWindow::setupDefaultMaterials() {

    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(QString("Exterior")));
    newItem->setText(1, QString("%1").arg(0));
    QColor color = QColor::fromRgb(0,0,0);
    newItem->setBackground(2, QBrush(color));

    int newMaterialIdx = 1;
    newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(QString("material%1").arg(newMaterialIdx)));
    newItem->setText(1, QString("%1").arg(newMaterialIdx));
    color = QColor::fromRgb(255,127,0);
    newItem->setBackground(2, QBrush(color));

    newMaterialIdx++;
    newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(QString("material%1").arg(newMaterialIdx)));
    newItem->setText(1, QString("%1").arg(newMaterialIdx));
    color = QColor::fromRgb(255,250,20);
    newItem->setBackground(2, QBrush(color));
}

// put them into the interface
void MainWindow::getMaterialsFromLabel() {
  if (!lab1)
    return; // nothing to do

  for (unsigned int i = 0; i < lab1->materialNames.size(); i++) {
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(lab1->materialNames.at(i)));
    newItem->setText(1, QString("%1").arg(i));
    QColor color = lab1->materialColors.at(i);
    newItem->setBackground(2, QBrush(color));
  }
}


// add new material
void MainWindow::on_pushButton_clicked()
{
    int newMaterialIdx = ui->treeWidget->topLevelItemCount();
    QString name = QString("material%1").arg(newMaterialIdx);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(name));
    newItem->setText(1, QString("%1").arg(newMaterialIdx));
    QColor color = QColorDialog::getColor();
    newItem->setBackground(2, QBrush(color));

    // add this material to lab1 as well
    if (lab1) {
      lab1->materialNames.push_back(name);
      lab1->materialColors.push_back(color);
    }
}

// delete the currently selected material
void MainWindow::on_pushButton_2_clicked()
{
    int index = ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->currentItem());
    delete ui->treeWidget->takeTopLevelItem(index);
}

void MainWindow::LoadImage() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
      ReadMGZ *reader = new ReadMGZ(fileName);
      std::vector<ScalarVolume> *volumes = reader->getVolume();
      if (!volumes || volumes->size() < 1) {
        fprintf(stderr, "Error: no volumes could be found");
        return;
      }

      if (volumes->size() == 4) {
        vol1 = (Volume *)volumes->at(0).convertToColorVolume(&volumes->at(0),
                                                  &volumes->at(1),
                                                  &volumes->at(2),
                                                  &volumes->at(3));
      } else if (volumes->size() == 3) {
        vol1 = (Volume *)volumes->at(0).convertToColorVolume(&volumes->at(0),
                                                  &volumes->at(1),
                                                  &volumes->at(2));
      } else if (volumes->size() == 1) {
        vol1 = (Volume *)&volumes->at(0);
      }

      if (volumes->size() > 1 && volumes->size() != 3 && volumes->size() != 4) {
        QMessageBox::information(this, tr("Image Segmentation Editor"),
                                 tr("More than one volume in file %1. Show the first volume only.").arg(fileName));
        fprintf(stderr, "Warning: several volumes found in the file, use only the first volume");
      }
      windowLevel[0] = vol1->autoWindowLevel[0];
      windowLevel[1] = vol1->autoWindowLevel[1];
      if (slicePosition.size() == 0) { // otherwise we have already a slice position
        slicePosition.push_back(vol1->size[0]/2);
        slicePosition.push_back(vol1->size[1]/2);
        slicePosition.push_back(vol1->size[2]/2);
        fprintf(stderr, "image with data range: %f %f ", vol1->range[0], vol1->range[1]);
      }

      updateActions();

      update();

      scaleImage(1.0);

      //showHistogram(vol1, ui->windowLevelHistogram);
    }
}

// draw the histogram
void MainWindow::showHistogram(Volume *vol1, QGraphicsView *gv) {
  if (vol1->hist.size() == 0)
    return; // nothing to do

  int w = gv->width();
  int h = gv->height();

  ulong cumsum = 0;
  for (unsigned int i = 0; i < vol1->hist.size(); i++) {
    cumsum += vol1->hist[i];
  }

  std::vector<double> histTmp(vol1->hist.size(), 0.0);
  for (unsigned int i = 0; i < histTmp.size(); i++) {
    histTmp[i] = log(vol1->hist[i]/(double)cumsum);
  }
  double cumsum2 = 0.0;
  for (unsigned int i = 0; i < histTmp.size(); i++) {
    cumsum2 = histTmp[i];
  }
  for (unsigned int i = 0; i < histTmp.size(); i++) {
    histTmp[i] = histTmp[i]/(double)cumsum2;
  }

  QGraphicsScene *scene = new QGraphicsScene();
  gv->setScene(scene);
  float wi = w/(double)(vol1->hist.size()-1);
  for (unsigned int i = 0; i < vol1->hist.size(); i+=3) {
     float x   = i * wi;
     float y   = h - (histTmp[i]*h);
     float val = histTmp[i]*h;
     scene->addRect(x, y, 3, val );
  }
  gv->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
  gv->setRenderHint(QPainter::Antialiasing, false);
  gv->show();
}

void MainWindow::LoadLabel() {
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open File"), QDir::currentPath());
  if (!fileName.isEmpty()) {
    ReadMGZ *reader = new ReadMGZ(fileName);
    std::vector<ScalarVolume> *volumes = reader->getVolume();
    if (!volumes || volumes->size() < 1) {
      fprintf(stderr, "Error: no volumes could be found");
      return;
    }

    if (volumes->size() > 1) {
      QMessageBox::information(this, tr("Image Segmentation Editor"),
                               tr("More than one volume in file %1. Show the first volume only.").arg(fileName));
      fprintf(stderr, "Warning: several volumes found in the file, use only the first volume");
    }
    lab1 = (Volume *)&volumes->at(0);
    windowLevelOverlay[0] = lab1->autoWindowLevel[0];
    windowLevelOverlay[1] = lab1->autoWindowLevel[1];

    getMaterialsFromLabel();
    update();

    hbuffer.resize( (ulong)lab1->size[0] * lab1->size[1] * lab1->size[2] );

  }
}

void MainWindow::SaveLabel() {
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Save File"), QDir::homePath());
  if (!fileName.isEmpty()) {
     ((ScalarVolume *)lab1)->saveAs(fileName);
  }
}


void MainWindow::CreateLabel() {
    if (!vol1) {
        // cannot be created without volume loaded first
        QMessageBox::information(this, tr("Image Segmentation Editor"),
                                 tr("Load a volume first."));
        return;
    }
    lab1 = (Volume*)new ScalarVolume(vol1->size, MyPrimType::UCHAR);
    memset(lab1->dataPtr, 0, lab1->size[0]*lab1->size[1]*lab1->size[2]);
    lab1->range[0] = 0; lab1->range[1] = 0;
    lab1->computeHist();
    getMaterialsFromLabel();
    hbuffer.resize( (ulong)lab1->size[0] * lab1->size[1] * lab1->size[2] );
}

void MainWindow::createActions() {
    ui->actionLoad_Image->setShortcut(tr("Ctrl+I"));
    connect(ui->actionLoad_Image, SIGNAL(triggered()), this, SLOT(LoadImage()));

    ui->actionSave_Label->setShortcut(tr("Ctrl+S"));
    connect(ui->actionSave_Label, SIGNAL(triggered()), this, SLOT(SaveLabel()));

    ui->actionLoad_Label->setShortcut(tr("Ctrl+L"));
    connect(ui->actionLoad_Label, SIGNAL(triggered()), this, SLOT(LoadLabel()));

    ui->actionCreate_New_Label->setShortcut(tr("Ctrl+N"));
    connect(ui->actionCreate_New_Label, SIGNAL(triggered()), this, SLOT(CreateLabel()));
}

void MainWindow::FillHighlight(int which) {
  fprintf(stderr, "fill in highlight");
  if (!hbuffer.size())
    return;

  // use flood fill scanline method
  // start at the first voxel for the current slide
  if (which == 0) {
    ulong offset = slicePosition[2] * (lab1->size[0]*lab1->size[1]);
    for (int j = 0; j < lab1->size[1]; j++) {
      bool inside = false;
      bool firstFlip = false;
      ulong start = -1;
      ulong end   = -1;
      for (int i = 0; i < lab1->size[0]; i++) {
        if (!inside) { // we are outside
          if (hbuffer[offset+j*lab1->size[0]+i] && !firstFlip) {
            firstFlip = true; // we wait for the second flip
          } else if (!hbuffer[offset+j*lab1->size[0]+i] && firstFlip) {
            start = offset+j*lab1->size[0]+i;
            inside = true;
          }
        } else { // we are inside
          if (hbuffer[offset+j*lab1->size[0]+i]) {
            end = offset+j*lab1->size[0]+i;
            inside = false;
            // now fill the area between start end end
            for (ulong k = start; k < end; k++) {
              hbuffer[k] = true;
            }
            // walk to the end of the border to be outside again
            for (ulong k = end; k < offset+(j+1)*lab1->size[0]; k++) {
              if (!hbuffer[k]) {
                i += (k-end);
                break;
              }
            }
            firstFlip = false;
          }
        }
      }
    }
    update();
  }

}

void MainWindow::updateActions() {
    //zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    //zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    //normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void MainWindow::normalSize() {
    ui->Image1->adjustSize();
    ui->Image2->adjustSize();
    ui->Image3->adjustSize();
    scaleFactor1 = 1.0;
    scaleFactor23 = 1.0;
}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor) {
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void MainWindow::scaleImage(double factor) {
  if (!vol1)
    return;

    Q_ASSERT(ui->Image1->pixmap());
    Q_ASSERT(ui->Image2->pixmap());
    Q_ASSERT(ui->Image3->pixmap());

    scaleFactor1 *= factor;
    scaleFactor23 *= factor;
    ui->Image1->resize(scaleFactor1  * ui->Image1->pixmap()->size());
    ui->Image2->resize(scaleFactor23 * ui->Image2->pixmap()->size());
    ui->Image3->resize(scaleFactor23 * ui->Image3->pixmap()->size());

    adjustScrollBar(ui->scrollArea_4->horizontalScrollBar(), factor);
    adjustScrollBar(ui->scrollArea_4->verticalScrollBar(), factor);

    adjustScrollBar(ui->scrollArea_2->horizontalScrollBar(), factor);
    adjustScrollBar(ui->scrollArea_2->verticalScrollBar(), factor);

    adjustScrollBar(ui->scrollArea_3->horizontalScrollBar(), factor);
    adjustScrollBar(ui->scrollArea_3->verticalScrollBar(), factor);

    //zoomInAct->setEnabled(scaleFactor < 3.0);
    //zoomOutAct->setEnabled(scaleFactor > 0.333);
}

// update the display
void MainWindow::update() {
    if (!vol1)
        return; // do nothing
    if (slicePosition.size() < 3)
        return;

    // normalize slice position
    if (slicePosition[0] < 0)
        slicePosition[0] = 0;
    if (slicePosition[1] < 0)
        slicePosition[1] = 0;
    if (slicePosition[2] < 0)
        slicePosition[2] = 0;
    if (slicePosition[0] > vol1->size[0]-1)
        slicePosition[0] = vol1->size[0]-1;
    if (slicePosition[1] > vol1->size[1]-1)
        slicePosition[1] = vol1->size[1]-1;
    if (slicePosition[2] > vol1->size[2]-1)
        slicePosition[2] = vol1->size[2]-1;

    // draw first image
    updateImage1(slicePosition[2]);
    updateImage2(slicePosition[1]);
    updateImage3(slicePosition[0]);

    ui->label->setText(QString("x: %1 y: %2 z: %3").arg(slicePosition[0]).arg(slicePosition[1]).arg(slicePosition[2]));
}

unsigned char * MainWindow::fillBuffer1(int pos, Volume *vol1, float alpha) {

  int numBytes = MyPrimType::getTypeSize(vol1->dataType);
  int numElements = vol1->elementLength;
  long offset = pos*(vol1->size[0]*vol1->size[1])*numBytes*numElements;

  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr + offset;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[1]);
  switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        if (vol1->elementLength == 1) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val; // blue
            buffer[i*4+1] = val; // green
            buffer[i*4+2] = val; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            buffer[i*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // blue
            buffer[i*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // green
            buffer[i*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data+=4;
          }
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        if (vol1->elementLength == 1) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            buffer[i*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // blue
            buffer[i*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // green
            buffer[i*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data+=4;
          }
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        if (vol1->elementLength == 1) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            buffer[i*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // blue
            buffer[i*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // green
            buffer[i*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data+=4;
          }
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        if (vol1->elementLength == 1) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            buffer[i*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // blue
            buffer[i*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // green
            buffer[i*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data+=4;
          }
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        if (vol1->elementLength == 1) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            buffer[i*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // blue
            buffer[i*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // green
            buffer[i*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data+=4;
          }
        }
        break;
      }
    case MyPrimType::FLOAT : {
        float *data = (float *)d;
        if (vol1->elementLength == 1) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
            buffer[i*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // blue
            buffer[i*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // green
            buffer[i*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data+=4;
          }
        }
        break;
      }

  }
  return buffer;
}

// create a label field from the volume
unsigned char * MainWindow::fillBuffer1AsColor(int pos, ScalarVolume *vol1, float alpha) {

  int numBytes = MyPrimType::getTypeSize(vol1->dataType);
  long offset = pos*(vol1->size[0]*vol1->size[1])*numBytes;

  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr + offset;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[1]);
  switch(vol1->dataType) {
  case MyPrimType::UCHAR :
  case MyPrimType::CHAR : {
      unsigned char *data = (unsigned char *)d;
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0]).blue(); // blue
          buffer[i*4+1] = vol1->materialColors.at(data[0]).green(); // green
          buffer[i*4+2] = vol1->materialColors.at(data[0]).red(); // red
          buffer[i*4+3] = data[0]==0?0:alpha; // alpha (0..254)
          data++;
      }
      break;
  }
  case MyPrimType::SHORT : {
      signed short *data = (signed short *)d;
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0]).blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0]).green();
          buffer[i*4+2] = vol1->materialColors.at(data[0]).red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
      }
      break;
  }
  case MyPrimType::USHORT : {
      unsigned short *data = (unsigned short *)d;
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0]).blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0]).green();
          buffer[i*4+2] = vol1->materialColors.at(data[0]).red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
      }
      break;
  }
  case MyPrimType::INT : {
      signed int *data = (signed int *)d;
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0]).blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0]).green();
          buffer[i*4+2] = vol1->materialColors.at(data[0]).red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
      }
      break;
  }
  case MyPrimType::UINT : {
      unsigned int *data = (unsigned int *)d;
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0]).blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0]).green();
          buffer[i*4+2] = vol1->materialColors.at(data[0]).red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
      }
      break;
  }
  default: {
        fprintf(stderr, "Error: this type is not allowed for label fields...");
      }
  }
  return buffer;
}

// create a label field from the volume
unsigned char * MainWindow::fillBuffer1FromHBuffer(int pos, ScalarVolume *vol1, float alpha) {

  if ((ulong)lab1->size[0]* lab1->size[1] * lab1->size[2] !=
      hbuffer.size()) {
    fprintf(stderr, "Error: hbuffer does not have same size as volume");
    return NULL;
  }
  int numBytes = MyPrimType::getTypeSize(vol1->dataType);
  long offset = pos*(vol1->size[0]*vol1->size[1])*numBytes;

  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr + offset;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[1]);
  switch(vol1->dataType) {
  case MyPrimType::UCHAR :
  case MyPrimType::CHAR : {
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = 0; // blue
          buffer[i*4+1] = 0; // green
          buffer[i*4+2] = (int)hbuffer[i+offset] * 255; // red
          buffer[i*4+3] = !hbuffer[i+offset]?0:alpha; // alpha (0..254)
      }
      break;
  }
  case MyPrimType::SHORT : {
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = 0;
          buffer[i*4+1] = 0;
          buffer[i*4+2] = (int)hbuffer[i+offset] * 255;
          buffer[i*4+3] = !hbuffer[i+offset]?0:alpha;
      }
      break;
  }
  case MyPrimType::USHORT : {
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = 0;
          buffer[i*4+1] = 0;
          buffer[i*4+2] = (int)hbuffer[i+offset] * 255;
          buffer[i*4+3] = !hbuffer[i+offset]?0:alpha;
      }
      break;
  }
  case MyPrimType::INT : {
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = 0;
          buffer[i*4+1] = 0;
          buffer[i*4+2] = (int)hbuffer[i+offset] * 255;
          buffer[i*4+3] = !hbuffer[i+offset]?0:alpha;
      }
      break;
  }
  case MyPrimType::UINT : {
      for (long i = 0; i < (long)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = 0;
          buffer[i*4+1] = 0;
          buffer[i*4+2] = (int)hbuffer[i+offset] * 255;
          buffer[i*4+3] = !hbuffer[i+offset]?0:alpha;
      }
      break;
  }
  default: {
        fprintf(stderr, "Error: this type is not allowed for label fields...");
      }
  }
  return buffer;
}


unsigned char *MainWindow::fillBuffer2(int pos, Volume *vol1, float alpha) {
  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[2]);
  switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned char *)d;
              data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned char *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }

        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (signed short *)d;
              data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (signed short *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }

        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned short *)d;
              data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned short *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }

        }
        break;
      }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (int *)d;
              data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (int *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }

        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned int *)d;
              data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned int *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }

        }
        break;
      }
    case MyPrimType::FLOAT : {
        float *data = (float *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (float *)d;
              data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (float *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }

        }
        break;
      }

  }
  return buffer;
}

unsigned char *MainWindow::fillBuffer2AsColor(int pos, ScalarVolume *vol1, float alpha) {
    // create a correct buffer for the image (ARGB32)
    unsigned char *d = vol1->dataPtr;
    unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[2]);
    switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        long count = 0;
        for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
                data = (unsigned char *)d;
                data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
                buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
                buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
                buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
                buffer[count*4+3] = data[0]==0?0:alpha;
                count++;
            }
        }
        break;
    }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        long count = 0;
        for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
                data = (signed short *)d;
                data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
                buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
                buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
                buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
                buffer[count*4+3] = data[0]==0?0:alpha;
                count++;
            }
        }
        break;
    }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        long count = 0;
        for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
                data = (unsigned short *)d;
                data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
                buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
                buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
                buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
                buffer[count*4+3] = data[0]==0?0:alpha;
                count++;
            }
        }
        break;
    }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        long count = 0;
        for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
                data = (int *)d;
                data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
                buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
                buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
                buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
                buffer[count*4+3] = data[0]==0?0:alpha;
                count++;
            }
        }
        break;
    }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        long count = 0;
        for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
                data = (unsigned int *)d;
                data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
                buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
                buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
                buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
                buffer[count*4+3] = data[0]==0?0:alpha;
                count++;
            }
        }
        break;
    }
    default: {
          fprintf(stderr, "Error: this type is not allowed as a label field");
    }
    }
    return buffer;
}

unsigned char *MainWindow::fillBuffer3(int pos, Volume *vol1, float alpha) {
  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[1] * vol1->size[2]);
  switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (unsigned char *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (unsigned char *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (signed short *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (signed short *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned short *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (unsigned short *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (int *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (int *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned int *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (unsigned int *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        }
        break;
      }
    case MyPrimType::FLOAT : {
        float *data = (float *)d;
        long count = 0;
        if (vol1->elementLength == 1) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[0]; i++) {
              data = (float *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+0] = val;
              buffer[count*4+1] = val;
              buffer[count*4+2] = val;
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        } else if (vol1->elementLength == 4) {
          for (long j = 0; j < vol1->size[2]; j++) {
            for (long i = 0; i < vol1->size[1]; i++) {
              data = (float *)d;
              data += 4*(j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos);
              buffer[count*4+0] = CLAMP((data[2] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+1] = CLAMP((data[1] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+2] = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
              buffer[count*4+3] = alpha;
              count++;
            }
          }
        }
        break;
      }

  }
  return buffer;
}

unsigned char *MainWindow::fillBuffer3AsColor(int pos, ScalarVolume *vol1, float alpha) {
  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[1] * vol1->size[2]);
  switch(vol1->dataType) {
  case MyPrimType::UCHAR :
  case MyPrimType::CHAR : {
      unsigned char *data = (unsigned char *)d;
      long count = 0;
      for (long j = 0; j < vol1->size[2]; j++) {
          for (long i = 0; i < vol1->size[1]; i++) {
              data = (unsigned char *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
              buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
              buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
              buffer[count*4+3] = data[0]==0?0:alpha;
              count++;
          }
      }
      break;
  }
  case MyPrimType::SHORT : {
      signed short *data = (signed short *)d;
      long count = 0;
      for (long j = 0; j < vol1->size[2]; j++) {
          for (long i = 0; i < vol1->size[0]; i++) {
              data = (signed short *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
              buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
              buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
              buffer[count*4+3] = data[0]==0?0:alpha;
              count++;
          }
      }
      break;
  }
  case MyPrimType::USHORT : {
      unsigned short *data = (unsigned short *)d;
      long count = 0;
      for (long j = 0; j < vol1->size[2]; j++) {
          for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned short *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
              buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
              buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
              buffer[count*4+3] = data[0]==0?0:alpha;
              count++;
          }
      }
      break;
  }
  case MyPrimType::INT : {
      signed int *data = (signed int *)d;
      long count = 0;
      for (long j = 0; j < vol1->size[2]; j++) {
          for (long i = 0; i < vol1->size[0]; i++) {
              data = (int *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
              buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
              buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
              buffer[count*4+3] = data[0]==0?0:alpha;
              count++;
          }
      }
      break;
  }
  case MyPrimType::UINT : {
      unsigned int *data = (unsigned int *)d;
      long count = 0;
      for (long j = 0; j < vol1->size[2]; j++) {
          for (long i = 0; i < vol1->size[0]; i++) {
              data = (unsigned int *)d;
              data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
              buffer[count*4+0] = vol1->materialColors.at(data[0]).blue();
              buffer[count*4+1] = vol1->materialColors.at(data[0]).green();
              buffer[count*4+2] = vol1->materialColors.at(data[0]).red();
              buffer[count*4+3] = data[0]==0?0:alpha;
              count++;
          }
      }
      break;
  }
default: {
        fprintf(stderr, "Error: this data type is not supported as a label");
      }
  }
  return buffer;
}



void MainWindow::updateImage1(int pos) {

  unsigned char *buffer = NULL;
  buffer = fillBuffer1(pos, vol1, 255);
  QImage imageVol1(buffer, vol1->size[0], vol1->size[1], QImage::Format_ARGB32);
  QImage resultImage(imageVol1); //  = QImage(imageVol1.size(), QImage::Format_ARGB32_Premultiplied);

  if (lab1 != NULL && lab1->elementLength == 1) {
    buffer = fillBuffer1AsColor(pos, (ScalarVolume *)lab1, 180);
    QImage imageLab1(buffer, lab1->size[0], lab1->size[1], QImage::Format_ARGB32);

    buffer = fillBuffer1FromHBuffer(pos, (ScalarVolume *)lab1, 180);
    QImage imageHBuffer1(buffer, lab1->size[0], lab1->size[1], QImage::Format_ARGB32);

    QPainter::CompositionMode mode = QPainter::CompositionMode_Overlay;

    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, imageVol1);
    painter.setCompositionMode(mode);
    painter.drawImage(0, 0, imageLab1);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, imageHBuffer1);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.fillRect(resultImage.rect(), Qt::white);
    painter.end();
  }
  //http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-imagecomposition.html

  ui->Image1->setPixmap(QPixmap::fromImage(resultImage));
  ui->scrollArea_4->setWidgetResizable(true);
}


void MainWindow::updateImage2(int pos) { // [x, 0, z]

  unsigned char *buffer = fillBuffer2(pos, vol1, 255);
  QImage imageVol1(buffer, vol1->size[0], vol1->size[2], QImage::Format_ARGB32);
  QImage resultImage(imageVol1);
  // resultImage = QImage(imageVol1.size(), QImage::Format_ARGB32_Premultiplied);

  if (lab1 != NULL && lab1->elementLength == 1) {
    buffer = fillBuffer2AsColor(pos, (ScalarVolume *)lab1, 128);
    QImage imageLab1(buffer, lab1->size[0], lab1->size[2], QImage::Format_ARGB32);

    QPainter::CompositionMode mode = QPainter::CompositionMode_Overlay;

    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, imageVol1);
    painter.setCompositionMode(mode);
    painter.drawImage(0, 0, imageLab1);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.fillRect(resultImage.rect(), Qt::white);
    painter.end();
  }
  //http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-imagecomposition.html

  ui->Image2->setPixmap(QPixmap::fromImage(resultImage));
  ui->scrollArea_2->setWidgetResizable(true);

}

void MainWindow::updateImage3(int pos) { // [0, y, z]

    unsigned char *buffer = fillBuffer3(pos, vol1, 255);
    QImage imageVol1(buffer, vol1->size[1], vol1->size[2], QImage::Format_ARGB32);
    QImage resultImage(imageVol1);

    if (lab1 != NULL && lab1->elementLength == 1) {
      buffer = fillBuffer3AsColor(pos, (ScalarVolume *)lab1, 128);
      QImage imageLab1(buffer, lab1->size[1], lab1->size[2], QImage::Format_ARGB32);

      QPainter::CompositionMode mode = QPainter::CompositionMode_Overlay;

      QPainter painter(&resultImage);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.fillRect(resultImage.rect(), Qt::transparent);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawImage(0, 0, imageVol1);
      painter.setCompositionMode(mode);
      painter.drawImage(0, 0, imageLab1);
      painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
      painter.fillRect(resultImage.rect(), Qt::white);
      painter.end();
    }
    //http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-imagecomposition.html

    ui->Image3->setPixmap(QPixmap::fromImage(resultImage));
    ui->scrollArea_3->setWidgetResizable(true);
}

// zoom in
void MainWindow::on_pushButton_3_clicked() {
    scaleImage(1.25);
}

void MainWindow::on_pushButton_4_clicked(){
    scaleImage(0.8);
}

// window level is on
void MainWindow::on_toolButton_toggled(bool checked) {
  if (checked) {
      toolWindowLevel = true;
      toolSegmentation = false;
    } else {
      toolWindowLevel = false;
      toolSegmentation = true;
    }
}

// segmentation on
void MainWindow::on_toolButton_2_toggled(bool checked) {
  if (checked) {
      toolWindowLevel = false;
      toolSegmentation = true;
    } else {
      toolWindowLevel = false;
      toolSegmentation = true;
    }
}

// reset the buffer
void MainWindow::on_toolButton_3_clicked()
{
  if (hbuffer.size()>0)
    hbuffer.reset();
  update();
}

// add to highlighted material
void MainWindow::on_toolButton_4_clicked()
{
  // what is the active materials index?
  int index = ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->currentItem());
  bool ok = false;
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if (!item) {
    fprintf(stderr, "Error: nothing selected");
    return;
  }
  int materialIdx = item->text(1).toInt(&ok);
  if (ok) {
    // we have a material that we want to add the currently highlighted voxels to
    for (ulong i = hbuffer.find_first(); i < hbuffer.npos; i = hbuffer.find_next(i)) {
      lab1->dataPtr[i] = materialIdx;
    }
    hbuffer.reset();
    update();
  }
}

// remove highlight from material
void MainWindow::on_toolButton_5_clicked()
{
  // we have Exterior that we want to add the currently highlighted voxels to
  for (ulong i = hbuffer.find_first(); i < hbuffer.npos; i = hbuffer.find_next(i)) {
    lab1->dataPtr[i] = 0; // exterior
  }
  hbuffer.reset();
  update();
}
