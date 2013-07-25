#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QColorDialog>
#include <QScrollBar>
#include <QBrush>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QEvent>
#include <QSettings>
#include "preferences.h"
#include <QtNetwork/QNetworkAccessManager>

#include "Types.h"
#include "Volume.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool mouseEvent(QObject *object, QMouseEvent *mouseEvent);
    bool myKeyPressEvent(QObject *object, QKeyEvent *keyEvent);
    bool myMouseReleaseEvent (QObject *object, QMouseEvent * e);
    void myMousePressEvent (QObject *object, QMouseEvent * e);
    bool myMouseWheelEvent (QObject *object, QWheelEvent * e);
    void myMouseButtonDblClick(QObject *object, QMouseEvent *mouseEvent);
    void redo();
    void undo();
    void LoadImageFromFile( QString fileName );
    void LoadLabelFromFile( QString fileName );
    std::vector<int> slicePosition;
    void update();
    void updateImage1(int pos);
    void updateImage2(int pos);
    void updateImage3(int pos);
    void snapshot( QString filename );
    void setMainWindowPos( int pos );
    void setCurrentWindowLevel( float a, float b );
    void disableAllButtonsBut( int t );

    polygon_type *ConvertHighlightToPolygon( int which );

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void SaveLabel(QString filename);
    void SaveLabelAskForName();
    void CreateLabel();
    void autoSave();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_toolButton_toggled(bool checked);

    void on_toolButton_2_toggled(bool checked);

    void on_toolButton_3_clicked();

    void on_toolButton_4_clicked();

    void on_toolButton_5_clicked();

    // slice orientation indicates which slice to fill
    void FillHighlight(int which);

    void on_toolButton_6_clicked();

    void on_toolButton_6_toggled(bool checked);

    void about();

    void createSnapshots();

    void on_pushButton_5_clicked();

    void on_zoomToggle_toggled(bool checked);

    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void finishedSlot(QNetworkReply* reply);

    void on_treeWidget_doubleClicked(const QModelIndex &index);

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_actionPreferences_triggered();

    void showBrightnessContrast();

    void on_scrollToggle_toggled(bool checked);

public slots:
    void loadRecentFile( QString fileName );
    void loadThisFile();
    void loadRecentLabel( QString fileName );
    void loadThisLabel();
    void showThisVolume( int idx );
    void LoadImage();
    void showThisVolumeAction();
    void LoadLabel();
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    double scaleFactor1;
    double scaleFactor23;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    void updateActions();

    QList<QTreeWidgetItem *> materials;

    void createActions();
    void setupDefaultMaterials();
    void getMaterialsFromLabel();
    void updateThumbnails();

    std::vector<Volume *> volumes;
    std::vector<Volume *> labels;

    Volume *vol1;
    Volume *lab1;
    QLabel *Image1;
    QLabel *Image2;
    QLabel *Image3;

    float windowLevel[2];
    float windowLevelOverlay[2];
    // each dataset has also a currentWindowLevel, if you change windowLevel change that one as well

    void normalSize();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void showHistogram(Volume *vol1, QGraphicsView *gv);
    int mousePressLocation[2];
    bool mouseIsDown;
    float windowLevelBefore[2];
    float scaleFactor1Before;
    float scaleFactor23Before;
    int slicePositionBefore[3];
    bool toolWindowLevel;
    bool toolSegmentation;

    unsigned char *fillBuffer1(int pos, Volume *vol1, float alpha=255);
    unsigned char *fillBuffer2(int pos, Volume *vol2, float alpha=255);
    unsigned char *fillBuffer3(int pos, Volume *vol3, float alpha=255);
    unsigned char *fillBuffer1AsColor(int pos, ScalarVolume *vol1, float alpha);
    unsigned char *fillBuffer2AsColor(int pos, ScalarVolume *vol1, float alpha);
    unsigned char *fillBuffer3AsColor(int pos, ScalarVolume *vol1, float alpha);

    unsigned char *fillBuffer1FromHBuffer(int pos, ScalarVolume *vol1, float alpha);
    unsigned char *fillBuffer2FromHBuffer(int pos, ScalarVolume *vol1, float alpha);
    unsigned char *fillBuffer3FromHBuffer(int pos, ScalarVolume *vol1, float alpha);

    // create storage for the highlight
    // add buffer add and remove to currently hightlighted material
    boost::dynamic_bitset<> hbuffer;

    void setHighlightBuffer(QObject *object, QMouseEvent *e);

    enum Tools {
      None,
      ContrastBrightness,
      BrushTool,
      MagicWandTool,
      ZoomTool,
      ScrollTool
    };

    Tools currentTool;
    int BrushToolWidth;
    QDir currentPath;
    void regionGrowing(int posx, int posy, int slice);
    void regionGrowing2(int posx, int posy, int slice);
    void regionGrowing3(int posx, int posy, int slice);

    bool showHighlights;

    // get a list of strings from internet
    QNetworkAccessManager* nam;
    QStringList fetchModel(QString aString);

    bool firstUndoStepDone;

    Preferences *preferencesDialog;
};

#endif // MAINWINDOW_H
