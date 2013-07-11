#include <QPainter>
#include <QShortcut>
#include <QDateTime>
#include <set>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>
#include <QGLWidget>
#include <QFileInfo>
#include <QGraphicsRectItem>
#include <QtNetwork/QNetworkRequest>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "undoredo.h"
#include "Types.h"
#include "exportsnapshots.h"
#include "readmgz.h"
#include "setbrightnesscontrast.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  setWindowTitle(tr("Image Segmentation Editor"));
  ui->setupUi(this);

  Image1 = new QLabel;
  Image1->setBackgroundRole(QPalette::Base);
  Image1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  Image1->setScaledContents(true);
  Image1->setFocusPolicy(Qt::WheelFocus);

  Image2 = new QLabel;
  Image2->setBackgroundRole(QPalette::Base);
  Image2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  Image2->setScaledContents(true);
  Image2->setFocusPolicy(Qt::WheelFocus);

  Image3 = new QLabel;
  Image3->setBackgroundRole(QPalette::Base);
  Image3->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  Image3->setScaledContents(true);
  Image3->setFocusPolicy(Qt::WheelFocus);


  ui->scrollArea_4->setWidget(Image1);
  ui->scrollArea_2->setWidget(Image2);
  ui->scrollArea_3->setWidget(Image3);

  ui->scrollArea_4->setBackgroundRole(QPalette::Dark);
  ui->scrollArea_2->setBackgroundRole(QPalette::Dark);
  ui->scrollArea_3->setBackgroundRole(QPalette::Dark);

  ui->treeWidget->setColumnCount(3);
  QStringList headerText;
  headerText << tr("Material") << tr("#") << tr("Color");
  ui->treeWidget->setHeaderLabels(headerText);
  ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);

  QList<int> sizes;
  sizes << 500 << 250;
  ui->splitter_2->setSizes(sizes);

  createActions();

  vol1 = NULL;
  lab1 = NULL;
  windowLevel[0] = 0;
  windowLevel[1] = 2400;
  windowLevelOverlay[0] = 0;
  windowLevelOverlay[1] = 255;
  mouseIsDown = false;

  scaleFactor1  = 4.8;
  scaleFactor23 = 4.8;

  qsrand(1234);
  currentTool = None;

  BrushToolWidth = 0;
  this->setWindowTitle(QString("Image Segmentation Editor"));

  QSettings settings;
  currentPath = settings.value("files/currentPath", QDir::currentPath()).toString();
  // currentPath = QDir::currentPath();

  showHighlights = true;
  nam = new QNetworkAccessManager(this);
  connect(nam, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(finishedSlot(QNetworkReply*)));

  firstUndoStepDone = false;

  preferencesDialog = new Preferences();

  // recent files
  QStringList files = preferencesDialog->getRecentFiles();
  for (int i = 0; i < files.size(); i++) {
    // create a new action
    QAction *action = new QAction((files[i]), this);
    if (!action)
      continue;
    action->setData( files[i] );
    //QLabel *label = new QLabel(files[i]);
    connect(action, SIGNAL(triggered()), this, SLOT(loadThisFile()));
    ui->menuRecent_Files->addAction(action);
  }
  // recent files
  files = preferencesDialog->getRecentLabels();
  for (int i = 0; i < files.size(); i++) {
    // create a new action
    QAction *action = new QAction((files[i]), this);
    if (!action)
      continue;
    action->setData( files[i] );
    //QLabel *label = new QLabel(files[i]);
    connect(action, SIGNAL(triggered()), this, SLOT(loadThisLabel()));
    ui->menuRecent_Label->addAction(action);
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}

// for the current menu item, get the filename and load it
void MainWindow::loadThisFile( ) {
  QAction *pAction = qobject_cast<QAction*>(sender());
  if (!pAction)
    return;
  QString filename = pAction->data().toString();
  loadRecentFile( filename );
}

// for the current menu item, get the filename and load it
void MainWindow::loadThisLabel( ) {
  QAction *pAction = qobject_cast<QAction*>(sender());
  if (!pAction)
    return;
  QString filename = pAction->data().toString();
  loadRecentLabel( filename );
}

void MainWindow::loadRecentFile( QString fileName ) {
  LoadImageFromFile( fileName );
}
void MainWindow::loadRecentLabel( QString fileName ) {
  LoadLabelFromFile( fileName );
}

void MainWindow::undo() {
  bool doTwoSteps = false;
  if (!firstUndoStepDone) { // we add the current state, so we can get back to it
    UndoRedo::getInstance().add(&hbuffer, lab1);
    firstUndoStepDone = true;
    doTwoSteps = true;
  }
  bool ok = UndoRedo::getInstance().prev();
  if (doTwoSteps) // we do a second prev to get over the current hump, something not right here still....
    ok = UndoRedo::getInstance().prev();


  if (ok) {
    if (UndoRedo::getInstance().isBoth()) {
      boost::dynamic_bitset<> *tmp = UndoRedo::getInstance().getBuffer();
      if (tmp)
        hbuffer = *tmp;
      if (UndoRedo::getInstance().getVolume())
        lab1 = UndoRedo::getInstance().getVolume();
    } else if (UndoRedo::getInstance().isBuffer()) {
      boost::dynamic_bitset<> *tmp = UndoRedo::getInstance().getBuffer();
      if (tmp)
        hbuffer = *tmp;
    } else if (UndoRedo::getInstance().isVolume()) {
      if (UndoRedo::getInstance().getVolume())
        lab1 = UndoRedo::getInstance().getVolume();
    }
    update();
  }
}

void MainWindow::redo() {
  bool ok = UndoRedo::getInstance().next();

  if (ok) {
    if (UndoRedo::getInstance().isBoth()) {
      boost::dynamic_bitset<> *tmp = UndoRedo::getInstance().getBuffer();
      if (tmp)
        hbuffer = *tmp;
      if (UndoRedo::getInstance().getVolume())
        lab1 = UndoRedo::getInstance().getVolume();
    } else if (UndoRedo::getInstance().isBuffer()) {
      boost::dynamic_bitset<> *tmp = UndoRedo::getInstance().getBuffer();
      if (tmp)
        hbuffer = *tmp;
    } else if (UndoRedo::getInstance().isVolume()) {
      if (UndoRedo::getInstance().getVolume())
        lab1 = UndoRedo::getInstance().getVolume();
    }
    update();
  }
}

// find out if we autosave is enabled, save the volume
void MainWindow::autoSave() {
  if (!lab1)
    return;
  if (preferencesDialog->isAutoSaveLabel()) {
    QFileInfo f(lab1->filename);
    if (f.exists() && f.isWritable()) {
       SaveLabel(lab1->filename); // save again
    } else {
      fprintf(stderr, "Error: auto-save could not write the file %s", lab1->filename.toLatin1().data());
    }
  }
}

bool MainWindow::myKeyPressEvent(QObject *object, QKeyEvent *keyEvent) {

  bool isShift = QApplication::keyboardModifiers() & Qt::ShiftModifier;
  bool isCtrl  = QApplication::keyboardModifiers() & Qt::ControlModifier;
  if (keyEvent->key() == Qt::Key_Z && isCtrl) {
    if (isShift)
      redo();
    else
      undo();
    keyEvent->accept();
    return true;
  }

  if (keyEvent->key() == Qt::Key_Tab && (object == Image1 ||
                                         object == Image2 ||
                                         object == Image3)) { // move through orientations
    std::vector<QWidget *> co(3);
    co[0] = ui->scrollArea_4->takeWidget();
    co[1] = ui->scrollArea_2->takeWidget();
    co[2] = ui->scrollArea_3->takeWidget();

    ui->scrollArea_4->setWidget(co[1]);
    ui->scrollArea_2->setWidget(co[2]);
    ui->scrollArea_3->setWidget(co[0]);
    ((QWidget*)object)->setFocus( Qt::TabFocusReason );

    keyEvent->accept();
    return true;
  }
  if (keyEvent->key() == Qt::Key_0 && (object == Image1 ||
                                       object == Image2 ||
                                       object == Image3)) {  // reset window level
    if (!vol1) {
      return false;
    }
    windowLevel[0] = vol1->autoWindowLevel[0];
    windowLevel[1] = vol1->autoWindowLevel[1];
    if (lab1) {
      windowLevelOverlay[0] = lab1->autoWindowLevel[0];
      windowLevelOverlay[1] = lab1->autoWindowLevel[1];
    }
    vol1->currentWindowLevel[0] = windowLevel[0];
    vol1->currentWindowLevel[1] = windowLevel[1];
    if (lab1) {
      lab1->currentWindowLevel[0] = windowLevelOverlay[0];
      lab1->currentWindowLevel[1] = windowLevelOverlay[1];
    }
    update();
    keyEvent->accept();
    return true;
  }
  if (keyEvent->key() == Qt::Key_Plus && (object == Image1 ||
                                          object == Image2 ||
                                          object == Image3)) {
    on_toolButton_4_clicked();
    keyEvent->accept();
    return true;
  }
  if (keyEvent->key() == Qt::Key_Minus && (object == Image1 ||
                                           object == Image2 ||
                                           object == Image3)) {
    on_toolButton_5_clicked();
    keyEvent->accept();
    return true;
  }
  if (keyEvent->key() == Qt::Key_Space && (object == Image1 ||
                                           object == Image2 ||
                                           object == Image3)) {
    showHighlights = !showHighlights;
    update();
    keyEvent->accept();
    return true;
  }
  if (keyEvent->key() == Qt::Key_C && (object == Image1 ||
                                       object == Image2 ||
                                       object == Image3)) {
    on_toolButton_3_clicked();
    keyEvent->accept();

    return true;
  }

  if (object != Image1 && object != Image2 && object != Image3) {
    QWidget::keyPressEvent(keyEvent);
    return false;
  }
  keyEvent->accept();

  // does not work because with Shift the key is a different key
  if (currentTool == MainWindow::BrushTool && keyEvent->key() == Qt::Key_0 && isShift)
    BrushToolWidth = 0;
  if (currentTool == MainWindow::BrushTool && keyEvent->key() == Qt::Key_1 && isShift)
    BrushToolWidth = 1;
  if (currentTool == MainWindow::BrushTool && keyEvent->key() == Qt::Key_2 && isShift)
    BrushToolWidth = 2;
  if (currentTool == MainWindow::BrushTool && keyEvent->key() == Qt::Key_3 && isShift)
    BrushToolWidth = 3;
  if (currentTool == MainWindow::BrushTool && keyEvent->key() == Qt::Key_4 && isShift)
    BrushToolWidth = 4;
  if (keyEvent->key() == Qt::Key_1 && !isShift) {
    if (volumes.size() >= 1) {
      vol1 = volumes[0];
      // restore window level
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_2 && !isShift) {
    if (volumes.size() >= 2) {
      vol1 = volumes[1];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_3 && !isShift) {
    if (volumes.size() >= 3) {
      vol1 = volumes[2];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_4 && !isShift) {
    if (volumes.size() >= 4) {
      vol1 = volumes[3];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_5 && !isShift) {
    if (volumes.size() >= 5) {
      vol1 = volumes[4];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_6 && !isShift) {
    if (volumes.size() >= 6) {
      vol1 = volumes[5];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_7 && !isShift) {
    if (volumes.size() >= 7) {
      vol1 = volumes[6];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_8 && !isShift) {
    if (volumes.size() >= 8) {
      vol1 = volumes[7];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_9 && !isShift) {
    if (volumes.size() >= 9) {
      vol1 = volumes[8];
      windowLevel[0] = vol1->currentWindowLevel[0];
      windowLevel[1] = vol1->currentWindowLevel[1];
      showHistogram(vol1, ui->windowLevelHistogram);
      update();
    }
  }

  if (keyEvent->key() == Qt::Key_1 && isCtrl) {
    if (labels.size() >= 1) {
      lab1 = labels[0];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_2 && isCtrl) {
    if (volumes.size() >= 2) {
      lab1 = labels[1];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_3 && isCtrl) {
    if (volumes.size() >= 3) {
      lab1 = labels[2];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_4 && isCtrl) {
    if (volumes.size() >= 4) {
      lab1 = labels[3];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_5 && isCtrl) {
    if (volumes.size() >= 5) {
      lab1 = labels[4];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_6 && isCtrl) {
    if (volumes.size() >= 6) {
      lab1 = labels[5];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_7 && isCtrl) {
    if (volumes.size() >= 7) {
      lab1 = labels[6];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_8 && isCtrl) {
    if (volumes.size() >= 8) {
      lab1 = labels[7];
      update();
    }
  }
  if (keyEvent->key() == Qt::Key_9 && isCtrl) {
    if (volumes.size() >= 9) {
      lab1 = labels[8];
      update();
    }
  }


  //fprintf(stderr, "MainWindow event received at: %d %d\n", keyEvent->pos().x(), mouseEvent->pos().y());
  if (object == Image1) {
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
  } else if (object == Image2) {
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
  } else if (object == Image3) {
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
  return true;
}

bool MainWindow::myMouseReleaseEvent ( QObject *object, QMouseEvent * e ) {
  //fprintf(stderr, "Mouse was released at: %d %d", e->pos().x(), e->pos().y() );
  mouseIsDown = false;

  if (object != Image1 && object != Image2 && object != Image3)
    return false;

  if (currentTool == MainWindow::BrushTool) {
    BrushToolWidth = preferencesDialog->brushSize();
    setHighlightBuffer(object, e);
    // add to undo
    UndoRedo::getInstance().add(&hbuffer);
    firstUndoStepDone = false;
    e->accept();
    return true;
  }
  if (currentTool == MainWindow::MagicWandTool) {
    e->accept();
    UndoRedo::getInstance().add(&hbuffer);
    firstUndoStepDone = false;
    return true;
  }
  return false;
}

void MainWindow::myMousePressEvent ( QObject *object, QMouseEvent * e ) {
  if (object != Image1 && object != Image2 && object != Image3 ) {
    return;
  }
  //fprintf(stderr, "Mouse was pressed at: %d %d", e->pos().x(), e->pos().y() );
  mousePressLocation[0] = e->pos().x();
  mousePressLocation[1] = e->pos().y();
  windowLevelBefore[0]  = windowLevel[0];
  windowLevelBefore[1]  = windowLevel[1];
  scaleFactor1Before    = scaleFactor1;
  scaleFactor23Before   = scaleFactor23;
  mouseIsDown = true;
  int posx = floor( e->pos().x() / scaleFactor1);
  int posy = floor( e->pos().y() / scaleFactor1);

  if (currentTool == MainWindow::BrushTool) {
    BrushToolWidth = preferencesDialog->brushSize();
    setHighlightBuffer(object, e);
  }
  if (currentTool == MainWindow::MagicWandTool && object == Image1) {
    regionGrowing(posx, posy, slicePosition[2]);
    update();
  } else if (currentTool == MainWindow::MagicWandTool && object == Image2) {
    posx = floor( e->pos().x() / scaleFactor23);
    posy = floor( e->pos().y() / scaleFactor23);
    regionGrowing2(posx, posy, slicePosition[1]);
    update();
  } else if (currentTool == MainWindow::MagicWandTool && object == Image3) {
    posx = floor( e->pos().x() / scaleFactor23);
    posy = floor( e->pos().y() / scaleFactor23);
    regionGrowing3(posx, posy, slicePosition[0]);
    update();
  }
  // add to undo
  //UndoRedo::getInstance().add(&hbuffer);
}

void MainWindow::setHighlightBuffer(QObject *object, QMouseEvent *e) {
  if (!lab1 || !hbuffer.size())
    return;

  if (hbuffer.size() != (size_t)lab1->size[0]*lab1->size[1]*lab1->size[2])
    return;
  // can be several tools create a brush first
  if (object == Image1) {
    int posx = floor( e->pos().x() / scaleFactor1);
    int posy = floor( e->pos().y() / scaleFactor1);

    bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;
    switch(BrushToolWidth) {
      case 1:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[slicePosition[2]*(lab1->size[0]*lab1->size[1]) + (posy+brushShape1[i])*lab1->size[0] + posx+brushShape1[i+1]] = !isCtrl;
        }
        break;
      case 2:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[slicePosition[2]*(lab1->size[0]*lab1->size[1]) + (posy+brushShape2[i])*lab1->size[0] + posx+brushShape2[i+1]] = !isCtrl;
        }
        break;
      case 3:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[slicePosition[2]*(lab1->size[0]*lab1->size[1]) + (posy+brushShape3[i])*lab1->size[0] + posx+brushShape3[i+1]] = !isCtrl;
        }
        break;
      case 4:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[slicePosition[2]*(lab1->size[0]*lab1->size[1]) + (posy+brushShape4[i])*lab1->size[0] + posx+brushShape4[i+1]] = !isCtrl;
        }
        break;
      default:
        fprintf(stderr, "error: brush shape not defined yet");
        break;
    }

  /*  int radius = ceil(BrushToolWidth/2.0);
    for (int i = posx-radius; i < posx+radius; i++) {    // 2
      for (int j = posy-radius; j < posy+radius; j++) {  // 1
        hbuffer[slicePosition[2]*(lab1->size[0]*lab1->size[1]) + j*lab1->size[0] + i] = !isCtrl;
      }
    } */
    update();
  } else if (object == Image2) {
    int posx = floor( e->pos().x() / scaleFactor23);
    int posy = floor( e->pos().y() / scaleFactor23);

    bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

    switch(BrushToolWidth) {
      case 1:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape1[i])*(lab1->size[0]*lab1->size[1]) + slicePosition[1]*lab1->size[0] + posx+brushShape1[i+1]] = !isCtrl;
        }
        break;
      case 2:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape2[i])*(lab1->size[0]*lab1->size[1]) + slicePosition[1]*lab1->size[0] + posx+brushShape2[i+1]] = !isCtrl;
        }
        break;
      case 3:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape3[i])*(lab1->size[0]*lab1->size[1]) + slicePosition[1]*lab1->size[0] + posx+brushShape3[i+1]] = !isCtrl;
        }
        break;
      case 4:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape4[i])*(lab1->size[0]*lab1->size[1]) + slicePosition[1]*lab1->size[0] + posx+brushShape4[i+1]] = !isCtrl;
        }
        break;
      default:
        fprintf(stderr, "error: brush shape not defined yet");
        break;
    }

/*    int radius = BrushToolWidth/2-1;
    for (int i = posx-radius; i <= posx+radius; i++) {  // 2
      for (int j = posy-radius; j <= posy+radius; j++) { // 0
        hbuffer[j*(lab1->size[0]*lab1->size[1]) + slicePosition[1]*lab1->size[0] + i] = !isCtrl;
      }
    } */
    update();

  } else if (object == Image3) {
    int posx = floor( e->pos().x() / scaleFactor23);
    int posy = floor( e->pos().y() / scaleFactor23);

    bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

    switch(BrushToolWidth) {
      case 1:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape1[i])*(lab1->size[0]*lab1->size[1]) + (posx+brushShape1[i+1])*lab1->size[0] + slicePosition[0]] = !isCtrl;
        }
        break;
      case 2:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape2[i])*(lab1->size[0]*lab1->size[1]) + (posx+brushShape2[i+1])*lab1->size[0] + slicePosition[0]] = !isCtrl;
        }
        break;
      case 3:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape3[i])*(lab1->size[0]*lab1->size[1]) + (posx+brushShape3[i+1])*lab1->size[0] + slicePosition[0]] = !isCtrl;
        }
        break;
      case 4:
        for (int i = 0; i < brushShapePixel[BrushToolWidth-1]*2; i+=2) {
          hbuffer[(posy+brushShape4[i])*(lab1->size[0]*lab1->size[1]) + (posx+brushShape4[i+1])*lab1->size[0] + slicePosition[0]] = !isCtrl;
        }
        break;
      default:
        fprintf(stderr, "error: brush shape not defined yet");
        break;
    }

    /* int radius = BrushToolWidth/2-1;
    for (int i = posx-radius; i <= posx+radius; i++) {   //
      for (int j = posy-radius; j <= posy+radius; j++) { //
        hbuffer[j*(lab1->size[0]*lab1->size[1]) + i*lab1->size[0] + slicePosition[0]] = !isCtrl;
      }
    } */
    update();
  }


}

bool MainWindow::mouseEvent(QObject *object, QMouseEvent *e) {
  if (object != Image1 && object != Image2 && object != Image3 ) {
    return false;
  }
  e->accept();

  //fprintf(stderr, "MainWindow event received at: %d %d\n", mouseEvent->pos().x(), mouseEvent->pos().y());
  if (slicePosition.size() == 3) {
    float x = floor(e->pos().x() / scaleFactor1);
    float y = floor(e->pos().y() / scaleFactor1);
    if (object == Image2 || object == Image3) {
      x = floor(e->pos().x() / scaleFactor23);
      y = floor(e->pos().y() / scaleFactor23);
    }

    ui->label->setText(QString("x: %1 y: %2 z: %3 (%4,%5)")
                       .arg(slicePosition[0])
        .arg(slicePosition[1])
        .arg(slicePosition[2])
        .arg( x )
        .arg( y ));
  }

  bool isShift = QApplication::keyboardModifiers() & Qt::ShiftModifier;
  // extract the intensity at the mouse location and show as text
  if (mouseIsDown && (currentTool == MainWindow::ContrastBrightness ||
                      isShift)) {
    float distx = (mousePressLocation[0] - e->pos().x())/scaleFactor1/200.0*(vol1->range[1]-vol1->range[0]);
    float disty = (mousePressLocation[1] - e->pos().y())/scaleFactor1/200.0*(vol1->range[1]-vol1->range[0]);
    if (object == Image2 || object == Image3) {
      distx = (mousePressLocation[0] - e->pos().x())/scaleFactor23/200.0*(vol1->range[1]-vol1->range[0]);
      disty = (mousePressLocation[1] - e->pos().y())/scaleFactor23/200.0*(vol1->range[1]-vol1->range[0]);
    }
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
    // fprintf(stderr, "window level change: %f %f", windowLevel[0], windowLevel[1]);
    vol1->currentWindowLevel[0] = windowLevel[0];
    vol1->currentWindowLevel[1] = windowLevel[1];

    ui->label->setText(QString("range: %1...%2")
        .arg( vol1->currentWindowLevel[0] )
        .arg( vol1->currentWindowLevel[1] ));

    update();
  } else if (mouseIsDown && currentTool == MainWindow::ZoomTool) {
    float disty = (mousePressLocation[1] - e->pos().y())/200.0;
    fprintf(stderr, "zoom changed: %f %f", scaleFactor1, disty);

    // set the factor explicitly
    scaleFactor1 = scaleFactor1Before+disty;
    scaleFactor23 = scaleFactor23Before+disty;
    scaleImage(1);

  } else if (mouseIsDown && currentTool == MainWindow::BrushTool) {
    if (!lab1)
      return false;
    BrushToolWidth = preferencesDialog->brushSize();
    setHighlightBuffer(object, e);
    // add to undo
    //UndoRedo::getInstance().add(&hbuffer);
  } else if (mouseIsDown && currentTool == MainWindow::MagicWandTool) {
    // can be several tools create a brush first
    if (lab1 && object == Image1) {
      int posx = floor( e->pos().x() / scaleFactor1);
      int posy = floor( e->pos().y() / scaleFactor1);

      regionGrowing(posx, posy, slicePosition[2]);
      update();
    } else if (lab1 && object == Image2) {
      int posx = floor( e->pos().x() / scaleFactor23);
      int posy = floor( e->pos().y() / scaleFactor23);

      regionGrowing2(posx, posy, slicePosition[1]);
      update();

    } else if (lab1 && object == Image3) {
      int posx = floor( e->pos().x() / scaleFactor23);
      int posy = floor( e->pos().y() / scaleFactor23);

      regionGrowing3(posx, posy, slicePosition[0]);
      update();
    }
    // add to undo
    //UndoRedo::getInstance().add(&hbuffer);
  }
  return true;
}

// add region to buffer
void MainWindow::regionGrowing(int posx, int posy, int slice) {
  if (!vol1 || posx < 0 || posx > vol1->size[0]-1 ||
      posy  < 0 || posy  > vol1->size[1]-1 ||
      slice < 0 || slice > vol1->size[2]-1 ||
      hbuffer.size() != (size_t)vol1->size[0]*vol1->size[1]*vol1->size[2])
    return;
  float fuzzy = 0.2*fabs(windowLevel[1]-windowLevel[0])
      * 0.2*fabs(windowLevel[1]-windowLevel[0]);
  size_t offset = (size_t)slice*vol1->size[0]*vol1->size[1];
  bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

  // either the current volume is a scalar or a color field
  if (vol1->elementLength == 1) {

    boost::dynamic_bitset<> done(vol1->size[0]*vol1->size[1]); // what was found
    std::vector<size_t> todo; // helper array to keep track what needs to be added
    switch(vol1->dataType) {
      case MyPrimType::UCHAR : {
          unsigned char *d = (unsigned char *)vol1->dataPtr + offset*vol1->elementLength;
          size_t idx = (size_t)posy*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y = floor(here/vol1->size[0]);
            int x = here-(y*vol1->size[0]);

            size_t n1 = (y+1)*vol1->size[0]+ x;
            size_t n2 = (y-1)*vol1->size[0]+ x;
            size_t n3 = (y)  *vol1->size[0]+(x+1);
            size_t n4 = (y)  *vol1->size[0]+(x-1);
            if (y+1 < vol1->size[1] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::SHORT : {
          short *d = (short *)vol1->dataPtr + offset*vol1->elementLength;
          size_t idx = (size_t)posy*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y = floor(here/vol1->size[0]);
            int x = here-(y*vol1->size[0]);

            size_t n1 = (y+1)*vol1->size[0]+ x;
            size_t n2 = (y-1)*vol1->size[0]+ x;
            size_t n3 = (y)  *vol1->size[0]+(x+1);
            size_t n4 = (y)  *vol1->size[0]+(x-1);
            if (y+1 < vol1->size[1] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::USHORT : {
          unsigned short *d = (unsigned short *)vol1->dataPtr + offset*vol1->elementLength;
          size_t idx = (size_t)posy*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y = floor(here/vol1->size[0]);
            int x = here-(y*vol1->size[0]);

            size_t n1 = (y+1)*vol1->size[0]+ x;
            size_t n2 = (y-1)*vol1->size[0]+ x;
            size_t n3 = (y)  *vol1->size[0]+(x+1);
            size_t n4 = (y)  *vol1->size[0]+(x-1);
            if (y+1 < vol1->size[1] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::FLOAT : {
          float *d = (float *)vol1->dataPtr + offset*vol1->elementLength;
          size_t idx = (size_t)posy*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y = floor(here/vol1->size[0]);
            int x = here-(y*vol1->size[0]);

            size_t n1 = (y+1)*vol1->size[0]+ x;
            size_t n2 = (y-1)*vol1->size[0]+ x;
            size_t n3 = (y)  *vol1->size[0]+(x+1);
            size_t n4 = (y)  *vol1->size[0]+(x-1);
            if (y+1 < vol1->size[1] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::INT : {
          signed int *d = (signed int *)vol1->dataPtr + offset*vol1->elementLength;
          size_t idx = posy*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y = floor(here/vol1->size[0]);
            int x = here-(y*vol1->size[0]);

            size_t n1 = (y+1)*vol1->size[0]+ x;
            size_t n2 = (y-1)*vol1->size[0]+ x;
            size_t n3 = (y)  *vol1->size[0]+(x+1);
            size_t n4 = (y)  *vol1->size[0]+(x-1);
            if (y+1 < vol1->size[1] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::UINT : {
          unsigned int *d = (unsigned int *)vol1->dataPtr + offset*vol1->elementLength;
          size_t idx = (size_t)posy*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y = floor(here/vol1->size[0]);
            int x = here-(y*vol1->size[0]);

            size_t n1 = (y+1)*vol1->size[0]+ x;
            size_t n2 = (y-1)*vol1->size[0]+ x;
            size_t n3 = (y)  *vol1->size[0]+(x+1);
            size_t n4 = (y)  *vol1->size[0]+(x-1);
            if (y+1 < vol1->size[1] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      default: {
          fprintf(stderr, "Error: this data type is not supported for this operation");
        }
    }
    // now copy everything in done into the hbuffer
    for (size_t i = 0; i < done.size(); i++) {
      if (done[i])
        hbuffer[offset+i] = !isCtrl;
    }

  } else if (vol1->elementLength == 4) {
    boost::dynamic_bitset<> done(vol1->size[0]*vol1->size[1]); // what was found
    std::vector<size_t> todo; // helper array to keep track what needs to be added
    switch(vol1->dataType) {
      case MyPrimType::UCHAR :
        unsigned char *d = (unsigned char *)vol1->dataPtr + offset*vol1->elementLength;
        size_t idx = (size_t)posy*vol1->size[0]+posx;
        float start[3];
        start[0] = d[4*idx+0];
        start[1] = d[4*idx+1];
        start[2] = d[4*idx+2];
        done.set(idx);
        todo.push_back(idx);
        for (size_t i = 0; i < todo.size(); i++) {
          size_t here = todo.at(i);
          int y = floor(here/vol1->size[0]);
          int x = here-(y*vol1->size[0]);

          size_t n1 = (size_t)(y+1)*vol1->size[0]+ x;
          size_t n2 = (size_t)(y-1)*vol1->size[0]+ x;
          size_t n3 = (size_t)(y)  *vol1->size[0]+(x+1);
          size_t n4 = (size_t)(y)  *vol1->size[0]+(x-1);
          if (y+1 < vol1->size[1] && !done[n1]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n1+0];
            h[1] = d[4*n1+1];
            h[2] = d[4*n1+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n1);
              done.set(n1);
            }
          }
          if (y-1 > -1 && !done[n2]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n2+0];
            h[1] = d[4*n2+1];
            h[2] = d[4*n2+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n2);
              done.set(n2);
            }
          }
          if (x+1 < vol1->size[0] && !done[n3]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n3+0];
            h[1] = d[4*n3+1];
            h[2] = d[4*n3+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n3);
              done.set(n3);
            }
          }
          if (x-1 > -1 && !done[n4]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n4+0];
            h[1] = d[4*n4+1];
            h[2] = d[4*n4+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n4);
              done.set(n4);
            }
          }
        }
        break;
    }
    // now copy everything in done into the hbuffer
    for (size_t i = 0; i < done.size(); i++) {
      if (done[i])
        hbuffer[offset+i] = !isCtrl;
    }
  } else {
    fprintf(stderr, "unknown element length for volume 1");
  }
}

// add region to buffer
void MainWindow::regionGrowing2(int posx, int posy, int slice) {  // slice is in direction y
  if (!vol1 || posx < 0 || posx > vol1->size[0]-1 ||
      posy  < 0 || posy  > vol1->size[2]-1 ||
      slice < 0 || slice > vol1->size[1]-1 ||
      hbuffer.size() != (size_t)vol1->size[0]*vol1->size[1]*vol1->size[2])
    return;
  float fuzzy = 0.2*fabs(windowLevel[1]-windowLevel[0])
      * 0.2*fabs(windowLevel[1]-windowLevel[0]);
  bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

  // either the current volume is a scalar or a color field
  if (vol1->elementLength == 1) {

    boost::dynamic_bitset<> done(vol1->size[0]*vol1->size[2]*vol1->size[1]); // what was found
    std::vector<size_t> todo; // helper array to keep track what needs to be added
    switch(vol1->dataType) {
      case MyPrimType::UCHAR : {
          unsigned char *d = (unsigned char *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int s  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            int x  = here - (y*vol1->size[0]*vol1->size[1] + s * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x+1);
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x-1);
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::FLOAT : {
          float *d = (float *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int s  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            int x  = here - (y*vol1->size[0]*vol1->size[1] + s * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x+1);
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x-1);
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::SHORT : {
          short *d = (short *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+posx;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int s  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            int x  = here - (y*vol1->size[0]*vol1->size[1] + s * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x+1);
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x-1);
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      default: {
          fprintf(stderr, "Error: this data type is not supported for this operation");
        }
    }
    // now copy everything in done into the hbuffer
    for (size_t i = 0; i < done.size(); i++) {
      if (done[i])
        hbuffer[i] = !isCtrl;
    }
  } else if (vol1->elementLength == 4) {
    boost::dynamic_bitset<> done(vol1->size[0]*vol1->size[1]*vol1->size[2]); // what was found
    std::vector<size_t> todo; // helper array to keep track what needs to be added
    switch(vol1->dataType) {
      case MyPrimType::UCHAR : {
          unsigned char *d = (unsigned char *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+posx;

          float start[3];
          start[0] = d[4*idx+0];
          start[1] = d[4*idx+1];
          start[2] = d[4*idx+2];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int s  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            int x  = here - (y*vol1->size[0]*vol1->size[1] + s * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x+1);
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x-1);
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n1+0];
              h[1] = d[4*n1+1];
              h[2] = d[4*n1+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n2+0];
              h[1] = d[4*n2+1];
              h[2] = d[4*n2+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n3+0];
              h[1] = d[4*n3+1];
              h[2] = d[4*n3+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n4+0];
              h[1] = d[4*n4+1];
              h[2] = d[4*n4+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
        }
        break;
      case MyPrimType::FLOAT : {
          float *d = (float *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+posx;

          float start[3];
          start[0] = d[4*idx+0];
          start[1] = d[4*idx+1];
          start[2] = d[4*idx+2];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int s  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            int x  = here - (y*vol1->size[0]*vol1->size[1] + s * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+x;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x+1);
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+slice*vol1->size[0]+(x-1);
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n1+0];
              h[1] = d[4*n1+1];
              h[2] = d[4*n1+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n2+0];
              h[1] = d[4*n2+1];
              h[2] = d[4*n2+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[0] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n3+0];
              h[1] = d[4*n3+1];
              h[2] = d[4*n3+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[3];
              h[0] = d[4*n4+0];
              h[1] = d[4*n4+1];
              h[2] = d[4*n4+2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) +
                   (start[1]-h[1]) * (start[1] - h[1]) +
                   (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
    }
    // now copy everything in done into the hbuffer
    for (size_t i = 0; i < done.size(); i++) {
      if (done[i])
        hbuffer[i] = !isCtrl;
    }
  } else {
    fprintf(stderr, "unknown element length for volume 1");
  }
}

// add region to buffer
void MainWindow::regionGrowing3(int posx, int posy, int slice) {  // slice is in direction x
  if (!vol1 || posx < 0 || posx > vol1->size[1]-1 ||
      posy  < 0 || posy  > vol1->size[2]-1 ||
      slice < 0 || slice > vol1->size[0]-1 ||
      hbuffer.size() != (size_t)vol1->size[0]*vol1->size[1]*vol1->size[2])
    return;
  float fuzzy = 0.2*fabs(windowLevel[1]-windowLevel[0])
      * 0.2*fabs(windowLevel[1]-windowLevel[0]);
  bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

  // either the current volume is a scalar or a color field
  if (vol1->elementLength == 1) {

    boost::dynamic_bitset<> done(vol1->size[0]*vol1->size[1]*vol1->size[2]); // what was found
    std::vector<size_t> todo; // helper array to keep track what needs to be added
    switch(vol1->dataType) {
      case MyPrimType::UCHAR : {
          unsigned char *d = (unsigned char *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+posx*vol1->size[0]+slice;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int x  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            //int s  = here - (y*vol1->size[0]*vol1->size[1] + x * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x+1)*vol1->size[0]+slice;
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x-1)*vol1->size[0]+slice;
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[1] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::SHORT : {
          short *d = (short *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+posx*vol1->size[0]+slice;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int x  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            //int s  = here - (y*vol1->size[0]*vol1->size[1] + x * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x+1)*vol1->size[0]+slice;
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x-1)*vol1->size[0]+slice;
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[1] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      case MyPrimType::FLOAT : {
          float *d = (float *)vol1->dataPtr;
          size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+posx*vol1->size[0]+slice;
          float start[1];
          start[0] = d[idx];
          done.set(idx);
          todo.push_back(idx);
          for (size_t i = 0; i < todo.size(); i++) {
            size_t here = todo.at(i);
            int y  = floor(here/(vol1->size[0]*vol1->size[1]));
            int x  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
            //int s  = here - (y*vol1->size[0]*vol1->size[1] + x * vol1->size[0]);

            size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
            size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
            size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x+1)*vol1->size[0]+slice;
            size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x-1)*vol1->size[0]+slice;
            if (y+1 < vol1->size[2] && !done[n1]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n1];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n1);
                done.set(n1);
              }
            }
            if (y-1 > -1 && !done[n2]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n2];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n2);
                done.set(n2);
              }
            }
            if (x+1 < vol1->size[1] && !done[n3]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n3];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n3);
                done.set(n3);
              }
            }
            if (x-1 > -1 && !done[n4]) {
              // check intensities at this location relative to first location
              float h[1];
              h[0] = d[n4];
              if ( (start[0]-h[0]) * (start[0] - h[0]) < fuzzy ) {
                todo.push_back(n4);
                done.set(n4);
              }
            }
          }
          break;
        }
      default: {
          fprintf(stderr, "Error: this data type is not supported for this operation");
        }
    }
    // now copy everything in done into the hbuffer
    for (size_t i = 0; i < done.size(); i++) {
      if (done[i])
        hbuffer[i] = !isCtrl;
    }
  } else if (vol1->elementLength == 4) {
    boost::dynamic_bitset<> done(vol1->size[0]*vol1->size[1]*vol1->size[2]); // what was found
    std::vector<size_t> todo; // helper array to keep track what needs to be added
    switch(vol1->dataType) {
      case MyPrimType::UCHAR : {
        unsigned char *d = (unsigned char *)vol1->dataPtr;
        size_t idx = (size_t)posy*vol1->size[0]*vol1->size[1]+posx*vol1->size[0]+slice;

        float start[3];
        start[0] = d[4*idx+0];
        start[1] = d[4*idx+1];
        start[2] = d[4*idx+2];
        done.set(idx);
        todo.push_back(idx);
        for (size_t i = 0; i < todo.size(); i++) {
          size_t here = todo.at(i);
          int y  = floor(here/(vol1->size[0]*vol1->size[1]));
          int x  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
          //int s  = here - (y*vol1->size[0]*vol1->size[1] + x * vol1->size[0]);

          size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
          size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
          size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x+1)*vol1->size[0]+slice;
          size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x-1)*vol1->size[0]+slice;
          if (y+1 < vol1->size[2] && !done[n1]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n1+0];
            h[1] = d[4*n1+1];
            h[2] = d[4*n1+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n1);
              done.set(n1);
            }
          }
          if (y-1 > -1 && !done[n2]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n2+0];
            h[1] = d[4*n2+1];
            h[2] = d[4*n2+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n2);
              done.set(n2);
            }
          }
          if (x+1 < vol1->size[1] && !done[n3]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n3+0];
            h[1] = d[4*n3+1];
            h[2] = d[4*n3+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n3);
              done.set(n3);
            }
          }
          if (x-1 > -1 && !done[n4]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n4+0];
            h[1] = d[4*n4+1];
            h[2] = d[4*n4+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n4);
              done.set(n4);
            }
          }
        }
        break;
        }
      case MyPrimType::FLOAT : {
        float *d = (float *)vol1->dataPtr;
        size_t idx = posy*vol1->size[0]*vol1->size[1]+posx*vol1->size[0]+slice;

        float start[3];
        start[0] = d[4*idx+0];
        start[1] = d[4*idx+1];
        start[2] = d[4*idx+2];
        done.set(idx);
        todo.push_back(idx);
        for (size_t i = 0; i < todo.size(); i++) {
          size_t here = todo.at(i);
          int y  = floor(here/(vol1->size[0]*vol1->size[1]));
          int x  = floor((here-(y*vol1->size[0]*vol1->size[1]))/vol1->size[0]);
          //int s  = here - (y*vol1->size[0]*vol1->size[1] + x * vol1->size[0]);

          size_t n1 = (size_t)(y+1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
          size_t n2 = (size_t)(y-1)*vol1->size[0]*vol1->size[1]+x*vol1->size[0]+slice;
          size_t n3 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x+1)*vol1->size[0]+slice;
          size_t n4 = (size_t)(y)  *vol1->size[0]*vol1->size[1]+(x-1)*vol1->size[0]+slice;
          if (y+1 < vol1->size[2] && !done[n1]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n1+0];
            h[1] = d[4*n1+1];
            h[2] = d[4*n1+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n1);
              done.set(n1);
            }
          }
          if (y-1 > -1 && !done[n2]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n2+0];
            h[1] = d[4*n2+1];
            h[2] = d[4*n2+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n2);
              done.set(n2);
            }
          }
          if (x+1 < vol1->size[1] && !done[n3]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n3+0];
            h[1] = d[4*n3+1];
            h[2] = d[4*n3+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n3);
              done.set(n3);
            }
          }
          if (x-1 > -1 && !done[n4]) {
            // check intensities at this location relative to first location
            float h[3];
            h[0] = d[4*n4+0];
            h[1] = d[4*n4+1];
            h[2] = d[4*n4+2];
            if ( (start[0]-h[0]) * (start[0] - h[0]) +
                 (start[1]-h[1]) * (start[1] - h[1]) +
                 (start[2]-h[2]) * (start[2] - h[2]) < fuzzy ) {
              todo.push_back(n4);
              done.set(n4);
            }
          }
        }
        break;
      }
    }
    // now copy everything in done into the hbuffer
    for (size_t i = 0; i < done.size(); i++) {
      if (done[i])
        hbuffer[i] = !isCtrl;
    }
  } else {
    fprintf(stderr, "unknown element length for volume 1");
  }
}



void MainWindow::myMouseButtonDblClick(QObject *object, QMouseEvent *mouseEvent) {
  if (object != Image1 && object != Image2 && object != Image3 )
    return;

  if (object == Image1) {
    slicePosition[0] = (int)floor( mouseEvent->pos().x() / scaleFactor1);
    slicePosition[1] = (int)floor( mouseEvent->pos().y() / scaleFactor1);
    // slicePosition[2] = (int)floor( mouseEvent->pos().x() / scaleFactor1 + 0.5);
    update();
  } else if (object == Image2) {
    slicePosition[0] = (int)floor( mouseEvent->pos().x() / scaleFactor23);
    //slicePosition[1] = (int)floor( mouseEvent->pos().y() / scaleFactor23);
    slicePosition[2] = (int)floor( mouseEvent->pos().y() / scaleFactor23);
    update();
  } else if (object == Image3) {
    // slicePosition[0] = (int)floor( mouseEvent->pos().x() / scaleFactor23);
    slicePosition[1] = (int)floor( mouseEvent->pos().x() / scaleFactor23);
    slicePosition[2] = (int)floor( mouseEvent->pos().y() / scaleFactor23);
    update();
  }
}

bool MainWindow::myMouseWheelEvent (QObject *object, QWheelEvent * e) {
  if (object != Image1 && object != Image2 && object != Image3 )
    return false;

  int numDegrees = e->delta() / 8;
  int numSteps = numDegrees / 15;

  if (object == Image1) {
    if (e->orientation() == Qt::Vertical) {
      slicePosition[2] += numSteps;
      update();
    }
    e->accept();
  } else if (object == Image2) {
    if (e->orientation() == Qt::Vertical) {
      slicePosition[1] += numSteps;
      update();
    }
    e->accept();
  } else if (object == Image3) {
    if (e->orientation() == Qt::Vertical) {
      slicePosition[0] += numSteps;
      update();
    }
    e->accept();
  }
  return true;
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

  // remove already existing entries first
  ui->treeWidget->clear();

  for (unsigned int i = 0; i < lab1->materialNames.size(); i++) {
    QColor *color = lab1->materialColors.at(i);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(lab1->materialNames.at(i)));
    newItem->setText(1, QString("%1").arg(i));
    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
    newItem->setBackground(2, QBrush(*color));
  }
}


// add new material
void MainWindow::on_pushButton_clicked()
{
  if (!lab1)
    CreateLabel();
  if (!lab1)
    return;

  int newMaterialIdx = ui->treeWidget->topLevelItemCount();
  QString name = QString("material%1").arg(newMaterialIdx);
  QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget, QStringList(name));
  newItem->setText(1, QString("%1").arg(newMaterialIdx));
  newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
  QColor *color = new QColor(QColorDialog::getColor());
  newItem->setBackground(2, QBrush(*color));

  // add this material to lab1 as well
  if (lab1) {
    lab1->materialNames.push_back(name);
    lab1->materialColors.push_back(color);
    // we need to update the entry if its getting changed as well
  }
}

// delete the currently selected material
void MainWindow::on_pushButton_2_clicked()
{
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if (!item)
    return;
  int idx = item->text(1).toInt();
  // remove material from label field as well
  if (idx < (int)lab1->materialNames.size()) {
    lab1->materialColors.erase(lab1->materialColors.begin()+idx);
    lab1->materialNames.erase(lab1->materialNames.begin()+idx);
  }

  int index = ui->treeWidget->indexOfTopLevelItem(item);
  delete ui->treeWidget->takeTopLevelItem(index);
}


QStringList MainWindow::fetchModel(QString aString)
{
/*    QString urlString("http://en.wikipedia.org/w/api.php?action=opensearch&search=");
    urlString.append(aString);
    urlString.append("&format=json&callback=spellcheck");

    QUrl url(urlString);
    nam->get(QNetworkRequest(url));

    return iStringList; */
  return QStringList(); // return empty list for now
}

void MainWindow::finishedSlot(QNetworkReply* reply)
{
/*    // Reading attributes of the reply
    // e.g. the HTTP status code
    QVariant statusCodeV =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    // Or the target URL if it was a redirect:
    QVariant redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    // no error received?
    if (reply->error() == QNetworkReply::NoError)
    {
        // read data from QNetworkReply here

        //  Reading bytes form the reply
        QByteArray bytes = reply->readAll();  // bytes
        QString string(bytes); // string

        ParseSpellResponse(string);

    }
    // Some http error received
    else
    {
        // handle errors here
    }

*/
}

void MainWindow::LoadImage() {

  /* QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open File"), currentPath.absolutePath()); */
  QFileDialog dialog(this);
  dialog.setDirectory( currentPath );
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilter(trUtf8("Splits (*.mgh)"));
  QStringList fileNames;
  if (dialog.exec()) {
    fileNames = dialog.selectedFiles();
    for ( int i = 0; i < fileNames.size(); i++) {
      QString fileName = fileNames[i];
      ui->statusbar->setEnabled(true);
      if (!fileName.isEmpty()) {
        LoadImageFromFile( fileName );
      }
    }
  }
}

void MainWindow::LoadImageFromFile( QString fileName ) {
    // set current path to new location
    currentPath.setPath( currentPath.filePath(fileName) );
    ReadMGZ *reader = new ReadMGZ(fileName);
    std::vector<ScalarVolume*> *rvolumes = reader->getVolume();
    if (!rvolumes || rvolumes->size() < 1) {
      fprintf(stderr, "Error: no volumes could be found");
      return;
    }

    ui->statusbar->showMessage(QString("Loading ") + fileName + QString("..."), 3000);
    if (rvolumes->size() == 4) {
      ScalarVolume *first = (ScalarVolume *)rvolumes->at(0);
      vol1 = (Volume *)first->convertToColorVolume(rvolumes->at(0),
                                                   rvolumes->at(1),
                                                   rvolumes->at(2),
                                                   rvolumes->at(3));
      free(rvolumes->at(0)->dataPtr);
      free(rvolumes->at(1)->dataPtr);
      free(rvolumes->at(2)->dataPtr);
      free(rvolumes->at(3)->dataPtr);
    } else if (rvolumes->size() == 3) {
      vol1 = (Volume *)rvolumes->at(0)->convertToColorVolume(rvolumes->at(0),
                                                             rvolumes->at(1),
                                                             rvolumes->at(2));
      free(rvolumes->at(0)->dataPtr);
      free(rvolumes->at(1)->dataPtr);
      free(rvolumes->at(2)->dataPtr);
    } else if (rvolumes->size() == 1) {
      vol1 = (Volume *)rvolumes->at(0);
    }

    if (rvolumes->size() > 1 && rvolumes->size() != 3 && rvolumes->size() != 4) {
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

    // append this to volumes if its not already there
    bool found = false;
    for (unsigned int i = 0; i < volumes.size(); i++) {
      if (vol1->filename == volumes[i]->filename)
        found = true; // already loaded
    }
    if (found == false)
      volumes.push_back(vol1);

    ui->windowLevelHistogram->setViewport( new QGLWidget() ); // makes the graphics window much much faster
    showHistogram(vol1, ui->windowLevelHistogram);
    preferencesDialog->addToRecentFiles(vol1->filename);
}

// draw the histogram
void MainWindow::showHistogram(Volume *vol1, QGraphicsView *gv) {
  if (vol1->hist.size() == 0)
    return; // nothing to do

  int w = gv->viewport()->width();
  int h = gv->viewport()->height();


  double cumsum = 0;
  for (unsigned int i = 0; i < vol1->hist.size(); i++) {
    float val = vol1->hist[i];
    cumsum += val;
  }

  std::vector<double> histTmp(vol1->hist.size(), 0.0);
  for (unsigned int i = 0; i < histTmp.size(); i++) {
    double v1 = (double)vol1->hist[i]/(double)cumsum;
    double v3 = log(v1*200+1);
    histTmp[i] = v3;
  }
  double max = histTmp[0];
  for (unsigned int i = 0; i < histTmp.size(); i++) {
    if (max < histTmp[i])
      max = histTmp[i];
  }
  for (unsigned int i = 0; i < histTmp.size(); i++) {
    histTmp[i] = histTmp[i]/(double)max;
  }

  QGraphicsScene *scene = new QGraphicsScene();
  gv->setScene(scene);
  unsigned int nbars = vol1->hist.size()/vol1->elementLength;
  float wi = w/(double)(nbars-1);
  for (int channel = 0; channel < vol1->elementLength; channel++) {
    for (unsigned int i = 0; i < nbars; i+=2) {
      float x   = i * wi;
      // float y   = histTmp[channel*vol1->hist.size()/vol1->elementLength+i]*h;
      float val = 10*histTmp[channel*nbars+i]*h;
      if (val > h)
        val = h;
      QGraphicsRectItem *item = new QGraphicsRectItem(x, h-val, 3, val);
      if (vol1->elementLength>1) {
        if (channel == 0)
          item->setBrush(QBrush(Qt::red));
        if (channel == 1)
          item->setBrush(QBrush(Qt::green));
        if (channel == 2)
          item->setBrush(QBrush(Qt::blue));
      } else {
        item->setBrush(QBrush(Qt::gray));
      }
      item->setPen( QPen(Qt::NoPen) );
      scene->addItem(item);
    }
  }
  gv->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
  gv->setRenderHint(QPainter::Antialiasing, false);
  gv->show();
}

void MainWindow::LoadLabel() {
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open File"), QDir::currentPath());
  if (fileName.isEmpty()) {
    fprintf(stderr, "no file selected");
    return;
  }
  LoadLabelFromFile(fileName);
}

void MainWindow::LoadLabelFromFile( QString fileName ) {

  ui->statusbar->showMessage(QString("Loading ") + fileName + QString("..."), 3000);
  ReadMGZ *reader = new ReadMGZ(fileName);
  std::vector<ScalarVolume *> *rvolumes = reader->getVolume();
  if (!rvolumes || rvolumes->size() < 1) {
    fprintf(stderr, "Error: no volumes could be found");
    return;
  }

  if (rvolumes->size() > 1) {
    QMessageBox::information(this, tr("Image Segmentation Editor"),
                             tr("More than one volume in file %1. Show the first volume only.").arg(fileName));
    fprintf(stderr, "Warning: several volumes found in the file, use only the first volume");
  }
  lab1 = (Volume *)rvolumes->at(0);
  windowLevelOverlay[0] = lab1->autoWindowLevel[0];
  windowLevelOverlay[1] = lab1->autoWindowLevel[1];

  getMaterialsFromLabel();
  update();


  hbuffer.resize( (size_t)lab1->size[0] * lab1->size[1] * lab1->size[2] );

  // append to list of loaded labels
  bool found = false;
  for (unsigned int i = 0; i < labels.size(); i++) {
    if (labels[i]->filename == lab1->filename)
      found = true;
  }
  if (found == false) {
    labels.push_back(lab1);
  }

  // read in the label field json file if there is one
  QJsonDocument desc = QJsonDocument();
  QFile file;
  file.setFileName(QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName() + ".json");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
     QTextStream streamFileIn(&file);
     QJsonParseError error;
     desc = QJsonDocument::fromJson(streamFileIn.readAll().toUtf8(), &error);
     file.close();
     if (desc.isNull()) {
       fprintf(stderr, "Error: parsing json file %s failed", file.fileName().toLatin1().data());
       return;
     }
     // put them into the file instead of the default onces
     lab1->materialNames.resize(0);
     lab1->materialColors.resize(0);
     QJsonObject obj = desc.object();
     QJsonArray materials = obj.value(QString("materials")).toArray();
     for (int i = 0; i < materials.size(); i++) {
       QJsonArray values = materials.at(i).toArray();
       QString name = values.at(0).toString();
       int idx = (int)values.at(1).toDouble();
       if (idx != i) {
         fprintf(stderr, "Error: the index in the file does not correspond to the order of the elements in the array");
       }
       QJsonArray color = values.at(2).toArray();
       double red   = color.at(0).toDouble();
       double green = color.at(1).toDouble();
       double blue  = color.at(2).toDouble();
       double alpha = color.at(3).toDouble();
       QColor *c = new QColor;
       c->setRgb((int)red, (int)green, (int)blue, (int)alpha);
       lab1->materialNames.push_back(name);
       lab1->materialColors.push_back(c);
     }
     getMaterialsFromLabel();
  }

  // add to undo
  UndoRedo::getInstance().add(&hbuffer, lab1);
  firstUndoStepDone = false;

  preferencesDialog->addToRecentLabels(lab1->filename);
}

void MainWindow::SaveLabelAskForName() {
  if (!lab1)
    return; // do nothing
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Save Label File"), currentPath.absolutePath());
  SaveLabel(fileName);
}

void MainWindow::SaveLabel(QString fileName) {
  if (!lab1)
    return; // do nothing
  if (!fileName.isEmpty()) {
    ((ScalarVolume *)lab1)->saveAs(fileName);

    // save the labels with their color as JSON file (same directory)
    // we can look for this file once we load a dataset again

    QJsonObject materials = QJsonObject();
    materials.insert("loadCmd", lab1->loadCmd);
    materials.insert("filename", lab1->filename);
    materials.insert("type", QString("labelfield"));
    QJsonArray dat = QJsonArray();
    for (unsigned int i = 0; i < lab1->materialNames.size(); i++) {
      QString name = lab1->materialNames.at(i);
      QColor *color = lab1->materialColors.at(i);
      int index = i;

      QJsonArray material = QJsonArray();
      material.insert(0, name);
      material.insert(1,index);
      QJsonArray c = QJsonArray();
      //int cc = color->red();
      //int cb = color->green();
      c.insert(0, color->red());
      c.insert(1, color->green());
      c.insert(2, color->blue());
      c.insert(3, color->alpha());
      material.insert(2,c);

      dat.append(material);
    }
    materials.insert("materials", dat);

    QJsonDocument desc = QJsonDocument(materials);
    QFile file;

    file.setFileName(QFileInfo(fileName).absolutePath() + QDir::separator() + QFileInfo(fileName).baseName() + ".json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
      fprintf(stderr, "Error: could not open json file to save labels");
      return;
    }
    QTextStream streamFileOut(&file);
    streamFileOut.setCodec("UTF-8");
    streamFileOut << desc.toJson();
    streamFileOut.flush();
    file.close();
  }
}

void MainWindow::snapshot( QString filename ) {
   // save the current image under this name
   // which image?
  if (ui->scrollArea_4->widget() == Image1) {
    const QPixmap *pm = Image1->pixmap();
    pm->save(filename, "png");
  } else if (ui->scrollArea_4->widget() == Image2) {
    const QPixmap *pm = Image2->pixmap();
    pm->save(filename, "png");
  } else if (ui->scrollArea_4->widget() == Image3) {
    const QPixmap *pm = Image3->pixmap();
    pm->save(filename, "png");
  }
}

void MainWindow::setMainWindowPos( int pos ) {
  if (ui->scrollArea_4->widget() == Image1) {
    slicePosition[2] = pos;
  } else if (ui->scrollArea_4->widget() == Image2) {
    slicePosition[1] = pos;
  } else if (ui->scrollArea_4->widget() == Image3) {
    slicePosition[0] = pos;
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
  lab1->filename = QString("label created on ") + QDateTime::currentDateTime().toString();
  memset(lab1->dataPtr, 0, lab1->size[0]*lab1->size[1]*lab1->size[2]);
  lab1->range[0] = 0; lab1->range[1] = 0;
  lab1->computeHist();
  getMaterialsFromLabel();

  hbuffer.resize( (size_t)lab1->size[0] * lab1->size[1] * lab1->size[2] );

  SaveLabelAskForName(); // store the label field for auto-save to work we need the file name

  // add to undo
  if (lab1) {
    UndoRedo::getInstance().add(&hbuffer, lab1);
    firstUndoStepDone = false;
  }
}

void MainWindow::createActions() {
  ui->actionLoad_Image->setShortcut(tr("Ctrl+I"));
  connect(ui->actionLoad_Image, SIGNAL(triggered()), this, SLOT(LoadImage()));

  ui->actionSave_Label->setShortcut(tr("Ctrl+S"));
  connect(ui->actionSave_Label, SIGNAL(triggered()), this, SLOT(SaveLabelAskForName()));

  ui->actionLoad_Label->setShortcut(tr("Ctrl+L"));
  connect(ui->actionLoad_Label, SIGNAL(triggered()), this, SLOT(LoadLabel()));

  ui->actionCreate_New_Label->setShortcut(tr("Ctrl+N"));
  connect(ui->actionCreate_New_Label, SIGNAL(triggered()), this, SLOT(CreateLabel()));

  ui->actionImage_Segmentation_Editor->setShortcut(tr("Ctrl+A"));
  connect(ui->actionImage_Segmentation_Editor, SIGNAL(triggered()), this, SLOT(about()));

  ui->actionSnapshots->setShortcut(tr("Ctrl+P"));
  connect(ui->actionSnapshots, SIGNAL(triggered()), this, SLOT(createSnapshots()));

  ui->actionSet_Brightness_Contrast->setShortcut(tr("Ctrl+-"));
  connect(ui->actionSet_Brightness_Contrast, SIGNAL(triggered()), this, SLOT(showBrightnessContrast()));
}

void MainWindow::setCurrentWindowLevel( float a, float b ) {
  if (!vol1)
    return;
  vol1->currentWindowLevel[0] = a;
  vol1->currentWindowLevel[1] = b;
  windowLevel[0] = a;
  windowLevel[1] = b;
}


// set brightness and contrast for the current window
void MainWindow::showBrightnessContrast() {
  if (!vol1) {
    return; // do nothing
  }
  SetBrightnessContrast *w = new SetBrightnessContrast(this);
  w->setRange(vol1->range[0], vol1->range[1]);
  w->setLowIntensity(vol1->currentWindowLevel[0]);
  w->setHighIntensity(vol1->currentWindowLevel[1]);
  w->show();
}

void MainWindow::createSnapshots() {
  // open the dialog
  if (!vol1)
    return; // we have to have a volume loaded

  ExportSnapshots *ss = new ExportSnapshots(this);
  if (ui->scrollArea_4->widget() == Image1) {
    ss->setMinMax( 0, vol1->size[2]-1 );
    ss->setStop( vol1->size[2]-1 );
  } else if (ui->scrollArea_4->widget() == Image2) {
    ss->setMinMax( 0, vol1->size[1]-1 );
    ss->setStop( vol1->size[1]-1 );
  } else if (ui->scrollArea_4->widget() == Image3) {
    ss->setMinMax( 0, vol1->size[0]-1 );
    ss->setStop( vol1->size[0]-1 );
  }
  ss->setStart( 0 );
  ss->setStep( 1 );

  ss->show();
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About Image Segmentation Editor"),
                     tr("<p>The <b>Image Segmentation Editor v0.7</b> is an application that"
                        " supports image segmentation on multi-modal image data."
                        "</p><br/>Hauke Bartsch, Dr. rer. nat. 2013"));
}

// we don't need this for filling the contour, this is only required to compute the
// line around a region of interest (like for displaying label borders instead of filled voxel)
polygon_type *MainWindow::ConvertHighlightToPolygon( int which ) {

  polygon_type *poly = new polygon_type();
  //boost::geometry::read_wkt(
  //     "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)"
  //     "(4.0 2.0, 4.2 1.4, 4.8 1.9, 4.4 2.2, 4.0 2.0))", poly);

  if (which == 0) {
    // we should use smallest containing bounding box to search in only
    // we should use thinning first, followed by scanline

    point_type p(4, 1);

    if (boost::geometry::within(p, *poly)) {
      // do something
    }
  } else if (which == 1) {

  } else if (which == 2) {

  }
  return poly;
}

void MainWindow::FillHighlight(int which) {
  //fprintf(stderr, "fill in highlight");
  if (!hbuffer.size())
    return;

  // add to (undo)
  UndoRedo::getInstance().add(&hbuffer);
  firstUndoStepDone = false;

  // use flood fill scanline method
  // start at the first voxel for the current slide
  if (which == 0) {    
    // calculate the smallest bounding box of the current buffer highlight

    // make a copy of that buffer region

    // calculate topology preserving thinning
    // https://github.com/bsdnoobz/zhang-suen-thinning/blob/master/thinning.cpp

    // calculate scan-line filling of the thinned contour

    // merge the resulting filled buffer with the original buffer


    size_t offset = slicePosition[2] * (lab1->size[0]*lab1->size[1]);
    for (int j = 0; j < lab1->size[1]; j++) {
      bool inside = false;
      bool firstFlip = false;
      size_t start = -1;
      size_t end   = -1;
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
            for (size_t k = start; k < end; k++) {
              hbuffer[k] = true;
            }
            // walk to the end of the border to be outside again
            for (size_t k = end; k < offset+(j+1)*lab1->size[0]; k++) {
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
  Image1->adjustSize();
  Image2->adjustSize();
  Image3->adjustSize();
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

  Q_ASSERT(Image1->pixmap());
  Q_ASSERT(Image2->pixmap());
  Q_ASSERT(Image3->pixmap());

  scaleFactor1 *= factor;
  scaleFactor23 *= factor;
  Image1->resize(scaleFactor1  * Image1->pixmap()->size());
  Image2->resize(scaleFactor23 * Image2->pixmap()->size());
  Image3->resize(scaleFactor23 * Image3->pixmap()->size());

  ui->scrollArea_4->setWidgetResizable(false);
  ui->scrollArea_2->setWidgetResizable(false);
  ui->scrollArea_3->setWidgetResizable(false);

  adjustScrollBar(ui->scrollArea_4->horizontalScrollBar(), scaleFactor1);
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
  // ui->label->setText(QString("x: %1 y: %2 z: %3").arg(slicePosition[0]).arg(slicePosition[1]).arg(slicePosition[2]));

  updateImage1(slicePosition[2]);
  updateImage2(slicePosition[1]);
  updateImage3(slicePosition[0]);
}

unsigned char * MainWindow::fillBuffer1(int pos, Volume *vol1, float alpha) {

  int numBytes = MyPrimType::getTypeSize(vol1->dataType);
  int numElements = vol1->elementLength;
  size_t offset = pos*(vol1->size[0]*vol1->size[1])*numBytes*numElements;

  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr + offset;
  unsigned char *buffer = (unsigned char*)malloc(4 * (size_t)vol1->size[0] * vol1->size[1]);
  switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        if (vol1->elementLength == 1) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val; // blue
            buffer[i*4+1] = val; // green
            buffer[i*4+2] = val; // red
            buffer[i*4+3] = alpha; // alpha (0..254)
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
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
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
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
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
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
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
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
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
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
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
            double val = CLAMP((data[0] - windowLevel[0])/(windowLevel[1]-windowLevel[0])*255, 0, 255);
            buffer[i*4+0] = val;
            buffer[i*4+1] = val;
            buffer[i*4+2] = val;
            buffer[i*4+3] = alpha;
            data++;
          }
        } else if (vol1->elementLength == 4) {
          for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
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
  size_t offset = pos*(vol1->size[0]*vol1->size[1])*numBytes;

  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr + offset;
  unsigned char *buffer = (unsigned char*)malloc(4 * (size_t)vol1->size[0] * vol1->size[1]);
  switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0])->blue(); // blue
          buffer[i*4+1] = vol1->materialColors.at(data[0])->green(); // green
          buffer[i*4+2] = vol1->materialColors.at(data[0])->red(); // red
          buffer[i*4+3] = data[0]==0?0:alpha; // alpha (0..254)
          data++;
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0])->blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0])->green();
          buffer[i*4+2] = vol1->materialColors.at(data[0])->red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0])->blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0])->green();
          buffer[i*4+2] = vol1->materialColors.at(data[0])->red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0])->blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0])->green();
          buffer[i*4+2] = vol1->materialColors.at(data[0])->red();
          buffer[i*4+3] = data[0]==0?0:alpha;
          data++;
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
          buffer[i*4+0] = vol1->materialColors.at(data[0])->blue();
          buffer[i*4+1] = vol1->materialColors.at(data[0])->green();
          buffer[i*4+2] = vol1->materialColors.at(data[0])->red();
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


unsigned char *MainWindow::fillBuffer2(int pos, Volume *vol1, float alpha) {
  // create a correct buffer for the image (ARGB32)
  unsigned char *d = vol1->dataPtr;
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[2]);
  switch(vol1->dataType) {
    case MyPrimType::UCHAR :
    case MyPrimType::CHAR : {
        unsigned char *data = (unsigned char *)d;
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (unsigned char *)d;
            data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (signed short *)d;
            data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (unsigned short *)d;
            data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (int *)d;
            data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (unsigned int *)d;
            data += j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
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


// create a label field from the volume
unsigned char * MainWindow::fillBuffer1FromHBuffer(int pos, ScalarVolume *vol1, float alpha) {

  if ((size_t)lab1->size[0]* lab1->size[1] * lab1->size[2] !=
      hbuffer.size()) {
    fprintf(stderr, "Error: hbuffer does not have same size as volume");
    return NULL;
  }
  int numBytes = MyPrimType::getTypeSize(vol1->dataType);
  size_t offset = pos*(vol1->size[0]*vol1->size[1])*numBytes;

  // create a correct buffer for the image (ARGB32)
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[1]);

  for (size_t i = 0; i < (size_t)vol1->size[0]*vol1->size[1]; i++) {
    buffer[i*4+0] = 0; // blue
    buffer[i*4+1] = 0; // green
    buffer[i*4+2] = (int)hbuffer[i+offset] * 255; // red
    buffer[i*4+3] = !hbuffer[i+offset]?0:alpha; // alpha (0..254)
  }

  return buffer;
}

// create a label field from the volume
unsigned char * MainWindow::fillBuffer2FromHBuffer(int pos, ScalarVolume *vol1, float alpha) {

  if ((size_t)lab1->size[0]* lab1->size[1] * lab1->size[2] !=
      hbuffer.size()) {
    fprintf(stderr, "Error: hbuffer does not have same size as volume");
    return NULL;
  }
  // create a correct buffer for the image (ARGB32)
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[0] * vol1->size[2]);

  for (int j = 0; j < vol1->size[2]; j++) {
    for (int i = 0; i < vol1->size[0]; i++) {
      size_t idx = j*(vol1->size[0]*vol1->size[1])+pos*vol1->size[0]+i;
      size_t idx2 = j*vol1->size[0]+i;
      buffer[idx2*4+0] = 0; // blue
      buffer[idx2*4+1] = 0; // green
      buffer[idx2*4+2] = (int)hbuffer[idx] * 255; // red
      buffer[idx2*4+3] = !hbuffer[idx]?0:alpha; // alpha (0..254)
    }
  }
  return buffer;
}

// create a label field from the volume
unsigned char * MainWindow::fillBuffer3FromHBuffer(int pos, ScalarVolume *vol1, float alpha) {

  if ((size_t)lab1->size[0]* lab1->size[1] * lab1->size[2] !=
      hbuffer.size()) {
    fprintf(stderr, "Error: hbuffer does not have same size as volume");
    return NULL;
  }

  // create a correct buffer for the image (ARGB32)
  unsigned char *buffer = (unsigned char*)malloc(4 * vol1->size[1] * vol1->size[2]);

  // todo: use loop over bitfield instead
  for (int j = 0; j < vol1->size[2]; j++) {
    for (int i = 0; i < vol1->size[1]; i++) {
      size_t idx = j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
      size_t idx2 = j*vol1->size[1]+i;
      buffer[idx2*4+0] = 0; // blue
      buffer[idx2*4+1] = 0; // green
      buffer[idx2*4+2] = (int)hbuffer[idx] * 255; // red
      buffer[idx2*4+3] = !hbuffer[idx]?0:alpha; // alpha (0..254)
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
        size_t count = 0;
        if (vol1->elementLength == 1) {
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[0]; i++) {
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
          for (size_t j = 0; j < vol1->size[2]; j++) {
            for (size_t i = 0; i < vol1->size[1]; i++) {
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
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[1]; i++) {
            data = (unsigned char *)d;
            data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::SHORT : {
        signed short *data = (signed short *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (signed short *)d;
            data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::USHORT : {
        unsigned short *data = (unsigned short *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (unsigned short *)d;
            data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::INT : {
        signed int *data = (signed int *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (int *)d;
            data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
            buffer[count*4+3] = data[0]==0?0:alpha;
            count++;
          }
        }
        break;
      }
    case MyPrimType::UINT : {
        unsigned int *data = (unsigned int *)d;
        size_t count = 0;
        for (size_t j = 0; j < vol1->size[2]; j++) {
          for (size_t i = 0; i < vol1->size[0]; i++) {
            data = (unsigned int *)d;
            data += j*(vol1->size[0]*vol1->size[1])+i*vol1->size[0]+pos;
            buffer[count*4+0] = vol1->materialColors.at(data[0])->blue();
            buffer[count*4+1] = vol1->materialColors.at(data[0])->green();
            buffer[count*4+2] = vol1->materialColors.at(data[0])->red();
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
  //static int lastPos;
  unsigned char *buffer1 = NULL;// buffer11;
  unsigned char *buffer2 = NULL;
  unsigned char *buffer3 = NULL;
  //if (lastPos != pos) {
  buffer1 = fillBuffer1(pos, vol1, 255);
  //   buffer11 = buffer1;
  //}
  if (!buffer1)
    return;
  QImage imageVol1(buffer1, vol1->size[0], vol1->size[1], QImage::Format_ARGB32);
  QImage resultImage(imageVol1); //  = QImage(imageVol1.size(), QImage::Format_ARGB32_Premultiplied);

  if (lab1 != NULL && lab1->elementLength == 1) {
    buffer2 = fillBuffer1AsColor(pos, (ScalarVolume *)lab1, 180);
    if (!buffer2)
      return;
    QImage imageLab1(buffer2, lab1->size[0], lab1->size[1], QImage::Format_ARGB32);

    buffer3 = fillBuffer1FromHBuffer(pos, (ScalarVolume *)lab1, 180);
    if (!buffer3)
      return;
    QImage imageHBuffer1(buffer3, lab1->size[0], lab1->size[1], QImage::Format_ARGB32);

    // QPainter::CompositionMode mode = QPainter::CompositionMode_Overlay;
    QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver;

    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, false);

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, imageVol1);
    if (showHighlights) {
      painter.setCompositionMode(mode);
      painter.drawImage(0, 0, imageLab1);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawImage(0, 0, imageHBuffer1);
    }
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.fillRect(resultImage.rect(), Qt::white);
    painter.end();
  }
  //http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-imagecomposition.html

  Image1->setPixmap(QPixmap::fromImage(resultImage));
  ui->scrollArea_4->setWidgetResizable(false);
  if (buffer1)
    free(buffer1);
  if (buffer2)
    free(buffer2);
  if (buffer3)
    free(buffer3);
}


void MainWindow::updateImage2(int pos) { // [x, 0, z]

  unsigned char *buffer1 = NULL;
  unsigned char *buffer2 = NULL;
  unsigned char *buffer3 = NULL;
  buffer1 = fillBuffer2(pos, vol1, 255);
  if (!buffer1)
    return;
  QImage imageVol1(buffer1, vol1->size[0], vol1->size[2], QImage::Format_ARGB32);
  QImage resultImage(imageVol1);
  // resultImage = QImage(imageVol1.size(), QImage::Format_ARGB32_Premultiplied);

  if (lab1 != NULL && lab1->elementLength == 1) {
    buffer2 = fillBuffer2AsColor(pos, (ScalarVolume *)lab1, 128);
    if (!buffer2)
      return;
    QImage imageLab1(buffer2, lab1->size[0], lab1->size[2], QImage::Format_ARGB32);

    buffer3 = fillBuffer2FromHBuffer(pos, (ScalarVolume *)lab1, 180);
    if (!buffer3)
      return;
    QImage imageHBuffer2(buffer3, lab1->size[0], lab1->size[2], QImage::Format_ARGB32);

    //QPainter::CompositionMode mode = QPainter::CompositionMode_Overlay;
    QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver;

    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, imageVol1);
    if (showHighlights) {
      painter.setCompositionMode(mode);
      painter.drawImage(0, 0, imageLab1);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawImage(0, 0, imageHBuffer2);
    }
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.fillRect(resultImage.rect(), Qt::white);
    painter.end();
  }
  //http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-imagecomposition.html

  Image2->setPixmap(QPixmap::fromImage(resultImage));
  ui->scrollArea_2->setWidgetResizable(false);
  if (buffer1)
    free(buffer1);
  if (buffer2)
    free(buffer2);
  if (buffer3)
    free(buffer3);
}

void MainWindow::updateImage3(int pos) { // [0, y, z]
  unsigned char *buffer1 = NULL;
  unsigned char *buffer2 = NULL;
  unsigned char *buffer3 = NULL;

  buffer1 = fillBuffer3(pos, vol1, 255);
  if (!buffer1)
    return;
  QImage imageVol1(buffer1, vol1->size[1], vol1->size[2], QImage::Format_ARGB32);
  QImage resultImage(imageVol1);

  if (lab1 != NULL && lab1->elementLength == 1) {
    buffer2 = fillBuffer3AsColor(pos, (ScalarVolume *)lab1, 128);
    if (!buffer2)
      return;
    QImage imageLab1(buffer2, lab1->size[1], lab1->size[2], QImage::Format_ARGB32);

    buffer3 = fillBuffer3FromHBuffer(pos, (ScalarVolume *)lab1, 180);
    if (!buffer3)
      return;
    QImage imageHBuffer3(buffer3, lab1->size[1], lab1->size[2], QImage::Format_ARGB32);

    // QPainter::CompositionMode mode = QPainter::CompositionMode_Overlay;
    QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver;

    QPainter painter(&resultImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(resultImage.rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0, 0, imageVol1);
    if (showHighlights) {
      painter.setCompositionMode(mode);
      painter.drawImage(0, 0, imageLab1);
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawImage(0, 0, imageHBuffer3);
    }
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.fillRect(resultImage.rect(), Qt::white);
    painter.end();
  }
  //http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-imagecomposition.html

  Image3->setPixmap(QPixmap::fromImage(resultImage));
  ui->scrollArea_3->setWidgetResizable(false);
  if (buffer1)
    free(buffer1);
  if (buffer2)
    free(buffer2);
  if (buffer3)
    free(buffer3);
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
    currentTool = MainWindow::ContrastBrightness;
  } else {
    currentTool = MainWindow::None;
  }
}

// segmentation on
void MainWindow::on_toolButton_2_toggled(bool checked) {
  if (checked) {
    currentTool = MainWindow::BrushTool;
  } else {
    currentTool = MainWindow::None;
  }
}

// reset the buffer
void MainWindow::on_toolButton_3_clicked()
{
  if (hbuffer.size()>0) {
    hbuffer.reset();
    UndoRedo::getInstance().add(&hbuffer);
    firstUndoStepDone = false;
  }
  update();

}

// add to highlighted material
void MainWindow::on_toolButton_4_clicked()
{
  // what is the active materials index?
  bool ok = false;
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if (!item) {
    fprintf(stderr, "Error: no material selected");
    return;
  }
  int materialIdx = item->text(1).toInt(&ok);
  if (ok) {
    // we have a material that we want to add the currently highlighted voxels to
    for (boost::dynamic_bitset<>::size_type i = hbuffer.find_first(); i != hbuffer.npos; i = hbuffer.find_next(i)) {
      lab1->dataPtr[i] = materialIdx;
    }

    hbuffer.reset();
    update();

    UndoRedo::getInstance().add(&hbuffer, lab1);
    autoSave();
    firstUndoStepDone = false;
  }
}

// remove highlight from material
void MainWindow::on_toolButton_5_clicked()
{
  if (!lab1)
    return;

  // we have Exterior that we want to add the currently highlighted voxels to
  for (boost::dynamic_bitset<>::size_type i = hbuffer.find_first(); i != hbuffer.npos; i = hbuffer.find_next(i)) {
    lab1->dataPtr[i] = 0; // exterior
  }

  hbuffer.reset();
  update();

  UndoRedo::getInstance().add(&hbuffer, lab1);
  autoSave();
  firstUndoStepDone = false;
}

// enable wand segmentation mode
void MainWindow::on_toolButton_6_clicked()
{
  // activeTool = Tools::MagicWandTool;
}

void MainWindow::on_toolButton_6_toggled(bool checked)
{
  if (checked)
    currentTool = MainWindow::MagicWandTool;
  else
    currentTool = MainWindow::None;
}

// Highlight/dehighlight material
void MainWindow::on_pushButton_5_clicked()
{
  bool ok = false;
  QTreeWidgetItem *item = ui->treeWidget->currentItem();
  if (!item) {
    fprintf(stderr, "Error: no material selected");
    return;
  }
  int materialIdx = item->text(1).toInt(&ok);
  if (ok) {

    // if not shift reset buffer first
    Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
    bool isSHIFT = keyMod.testFlag(Qt::ShiftModifier);
    if (!isSHIFT) {
      hbuffer.reset();
    }

    // we want to add the currently highlighted material to the buffer
    for (size_t i = 0; i < (size_t)lab1->size[0] * lab1->size[1] * lab1->size[2]; i++) {
      if (lab1->dataPtr[i] == materialIdx)
        hbuffer[i] = true;
    }
    update();

    UndoRedo::getInstance().add(&hbuffer);
    firstUndoStepDone = false;
  }

}

// change the zoom using the mouse
void MainWindow::on_zoomToggle_toggled(bool checked)
{
  if (checked) {
    currentTool = MainWindow::ZoomTool;
  } else {
    currentTool = MainWindow::None;
  }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (1 /*userReallyWantsToQuit() */ ) {
    QSettings settings;
    settings.setValue("files/currentPath", currentPath.absolutePath());

    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
  if (!lab1) {
    return;
  }

  // copy the change back to the lab1 structure
  int idx = item->text(1).toInt();
  QColor color = item->backgroundColor(2);
  QString name = item->text(0);
  if (idx < lab1->materialNames.size()) { // material exists, so change it, if it does not exist, wait
    lab1->materialNames[idx] = name;
    lab1->materialColors[idx] = new QColor(color);
  }
}

//double click on tree list item, start editing color
void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
  if (column == 2) {
     QColor old = item->backgroundColor(2);
     QColor color = QColorDialog::getColor(old);
     item->setBackground(2, QBrush(color));
  }
}

// this function is only called on already existing entries in the tree
void MainWindow::on_treeWidget_doubleClicked(const QModelIndex &index)
{
    // user double-clicked on an empty entry, what is the index?
//  if (index.isValid()) {
//    on_pushButton_clicked();
//  }
}

// undo
void MainWindow::on_actionUndo_triggered()
{
  undo();
}

// redo
void MainWindow::on_actionRedo_triggered()
{
  redo();
}

// open the preferences window
void MainWindow::on_actionPreferences_triggered()
{
  if (preferencesDialog)
    preferencesDialog->show();
}
