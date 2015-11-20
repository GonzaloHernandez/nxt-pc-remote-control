#ifndef WINDOW_H
#define WINDOW_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QProgressBar>
#include <QMenu>
#include <QFile>
#include <QThread>

#include <network.h>
#include <idiom.h>

#define len(x) sizeof(x)/sizeof(byte)
#define non(x) (byte)-(x)

/** ========================================================================
 * @brief MyButton class overload to QPushButton due to, was necesary do
 * programming with especial events.
 */
class MyButton : public QPushButton {
  Q_OBJECT
public:

  /** ----------------------------------------------------------------------
   * @brief MyButton constructor transport label to superclass
   */
  MyButton(QString label) : QPushButton(label) {
  }
signals:

  /** ----------------------------------------------------------------------
   * @brief This is an abstract method to rightClick() programming event
   */
  void rightClick(QPoint);

protected:

  /** ----------------------------------------------------------------------
   * @brief keyPressEvent create a keyboard press event
   */
  void keyPressEvent(QKeyEvent *ev) {
    ev->ignore();
  }

  /** ----------------------------------------------------------------------
   * @brief mousePressEvent create a mouse click event
   */
  void mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::RightButton) {
      emit rightClick(ev->pos());
    }
    else if (ev->button() == Qt::LeftButton) {
      emit clicked();
    }
  }
};

/** ========================================================================
 * @brief MyLabel class overload to QLabel due to, was necesary do
 * programming with especial events.
 */
class MyLabel : public QLabel {
  Q_OBJECT
signals:

  /** ----------------------------------------------------------------------
   * @brief This is an abstract method to rightClick() programming event
   */
  void rightClick(QPoint);

  /** ----------------------------------------------------------------------
   * @brief This is an abstract method to clicked() programming event
   */
  void clicked(bool);

protected:

  /** ----------------------------------------------------------------------
   * @brief mousePressEvent create a mouse click event
   */
  void mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::RightButton) {
      emit rightClick(ev->pos());
    }
    else {
      emit clicked(false);
    }
  }
};

/** ========================================================================
 * @brief Thread class is implemented for fix disable GUI programmed in
 * another release of this applications.
 */
class Thread : public QThread {
  Q_OBJECT
private:
  QComboBox* devices;
  Network* net;
  int operation;

signals:

  /** ----------------------------------------------------------------------
   * @brief Creating events to after running.
   */
  void scanPerformed(int);
  void connectPerformed(bool);

public:
  /** ----------------------------------------------------------------------
   * @brief Thread constructor receive device combo and network references
   * to allow work with them.
   */
  Thread(QComboBox* combo,Network* n,int op)
    : devices(combo), net(n), operation(op)  {
  }

  /** ----------------------------------------------------------------------
   * @brief run method has actions to exec in thread.
   */
  void run() {
    switch (operation) {
    case 1:
      try {
        QStringList s = net->scanDevices();
        devices->clear();
        devices->addItems(s);
        emit scanPerformed(0);
      }
      catch(int e) {
        devices->clear();
        emit scanPerformed(e);
      }
      break;
    case 2:
      bool state = net->bind(devices->currentText().left(17));
      emit connectPerformed(state);
      break;
    }
  }
};

/** ========================================================================
 * @brief Window class is the main class en this project, because has the
 * GUI functions to intercommunicate two actors.  NXT PC Remote Control and
 * LEGO gameer.
 */
class Window : public QFrame {
  Q_OBJECT
private:
  byte          power;
  bool          lowswitch;
  byte          powerlow;
  MyButton      *scan,*bind;
  QComboBox     *devices;
  MyLabel       *info;
  Network       *net;
  QProgressBar  *lowspeed,*highspeed;
  QMenu         *menu;
  QMenu         *recents,*selectidiom;
  Idiom         idiom;
  Thread        *t;

  /** ----------------------------------------------------------------------
   * @brief loadSettings method set de initial profiles to NXT PC Remote
   * Control using source file ".nxt-pc-remote-control.cfg"
   */
  void loadSettings() {
    QFile f(".nxt-pc-remote-control.cfg");
    f.open(QIODevice::ReadOnly);
    if (!f.isOpen()) {
       idiom.setIdiomType(ENG);
       return;
    }
    QString data = f.readLine().data();
    idiom.setIdiomType(data=="eng\n"?ENG:SPA);
    QString data2 = f.readLine();
    int recentsCount = data2.toInt();
    for (int i=0; i<recentsCount; i++) {
      data = f.readLine().data();
      recents->addAction(data);
    }
    refreshIdiom();
    f.close();
  }

  /** ----------------------------------------------------------------------
   * @brief addRecent method admin de cache of connections worked
   */
  void addRecent(QString data) {
    data+="\n";
    foreach (QAction* action, recents->actions()) {
      if (action->text() == data)return;
    }
    recents->addAction(data);
  }

  /** ----------------------------------------------------------------------
   * @brief saveSettings save current application settings into a file
   */
  void saveSettings() {
    QFile f(".nxt-pc-remote-control.cfg");
    f.open(QIODevice::WriteOnly);
    if (!f.isOpen()) return;
    f.write( idiom.getIdiomType()==ENG?"eng\n":"spa\n" );
    char size[4];
    sprintf( size,"%d\n", recents->actions().length() );
    f.write( size );
    foreach (QAction* action, recents->actions()) {
      f.write( action->text().toStdString().c_str() );
    }
    f.close();
  }

  /** ----------------------------------------------------------------------
   * @brief refreshIdiom method update idiom of application.
   */
  void refreshIdiom() {
    setWindowTitle(idiom.getWindowTitle());
    scan->setText(idiom.getScanButtonLabel());
    scan->isEnabled() ? bind->setText(idiom.getConnectButtonLabel()) :
                        bind->setText(idiom.getDisconnectButtonLabel());
    info->setPixmap(QPixmap(idiom.getImageInfo()));
    selectidiom->actions().at(0)->setText(idiom.getMenuEnglish());
    selectidiom->actions().at(1)->setText(idiom.getMenuSpanish());
    menu->actions().at(0)->setText(idiom.getMenuRecentConnections());
    menu->actions().at(1)->setText(idiom.getMenuClearConnections());
    menu->actions().at(3)->setText(idiom.getMenuSelectIdiom());
    menu->actions().at(5)->setText(idiom.getMenuAbout());
  }

public:

  /** ----------------------------------------------------------------------
   * @brief Window constructor launch application saving information in its
   * attribute, additionally, update GUI presentation.
   */
  Window(): power(0x55), lowswitch(false), powerlow(0x3E) {
    setWindowTitle(idiom.getWindowTitle());
    resize(250,100);

    scan      = new MyButton(idiom.getScanButtonLabel());
    devices   = new QComboBox();
    bind      = new MyButton(idiom.getConnectButtonLabel());
    info      = new MyLabel();
    lowspeed  = new QProgressBar();
    highspeed = new QProgressBar();
    menu      = new QMenu();

    QHBoxLayout* buttons = new QHBoxLayout();
    QVBoxLayout* layout = new QVBoxLayout();

    setLayout(layout);
    buttons->addWidget(scan);
    buttons->addWidget(bind);
    layout->addLayout(buttons);
    layout->addWidget(devices);
    layout->addWidget(info);
    layout->addWidget(highspeed);
    layout->addWidget(lowspeed);

    devices->setEnabled(false);
    bind->setEnabled(false);
    info->setPixmap(QPixmap(idiom.getImageInfo()));
    setStyleSheet("QFrame{background-color:white}");
    lowspeed->setMinimum(0x32);
    lowspeed->setMaximum(0x64);
    highspeed->setMinimum(0x32);
    highspeed->setMaximum(0x64);
    lowspeed->setValue(powerlow);
    highspeed->setValue(power);

    setFixedSize(278,438);
    recents = new QMenu(idiom.getMenuRecentConnections());
    selectidiom = new QMenu(idiom.getMenuSelectIdiom());
    menu->addMenu(recents);
    menu->addAction(idiom.getMenuClearConnections());
    menu->addSeparator();
    menu->addMenu(selectidiom);
    menu->addSeparator();
    menu->addAction(idiom.getMenuAbout());
    selectidiom->addAction(idiom.getMenuEnglish());
    selectidiom->addAction(idiom.getMenuSpanish());

    loadSettings();
    net = new Network();

    connect(scan,SIGNAL(clicked()),this,SLOT(scanDevices()));
    connect(bind,SIGNAL(clicked()),this,SLOT(connectDevice()));
    connect(info,SIGNAL(rightClick(QPoint)),this,SLOT(popMenu(QPoint)));
    connect(menu,SIGNAL(triggered(QAction*)),this,SLOT(menuOption(QAction*)));
    connect(recents,SIGNAL(triggered(QAction*)),this,SLOT(recentSelection(QAction*)));
    connect(selectidiom,SIGNAL(triggered(QAction*)),this,SLOT(changeIdiom(QAction*)));
    connect(info,SIGNAL(clicked(bool)),this,SLOT(showAbout(bool)));
  }

  /** ----------------------------------------------------------------------
   * @brief Window destructor to exec saving settings and clear memory
   */
  ~Window() {
    saveSettings();
    delete net;
  }

protected:

  /** ----------------------------------------------------------------------
   * @brief keyPressEvent exec commands when user is play with NXT PC
   * Remote Control.
   */
  void keyPressEvent(QKeyEvent *event) {
    if (bind->text() == idiom.connectButtonLabel) return;
    if (!event->isAutoRepeat()) {
      switch (event->key()) {

        case Qt::Key_Alt: {
          lowswitch = true;
          break;
        }

        case Qt::Key_B: {
          byte bytes[] = { 0x03, 0x0B, 0x02, 0xF4, 0x01 };
          Telegram t;
          t.append(bytes, len(bytes));
          net->directCommand(t);
          break;
        }

        case Qt::Key_Up : {
          byte bytes3[] = { 0x04, 0x01, lowswitch?non(powerlow):non(power),
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes3, len(bytes3));

          byte bytes4[] = { 0x04, 0x02, lowswitch?non(powerlow):non(power),
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes4, len(bytes4));
          break;
        }

        case Qt::Key_Down : {
          byte bytes3[] = { 0x04, 0x01, lowswitch?powerlow:power,
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes3, len(bytes3));

          byte bytes4[] = { 0x04, 0x02, lowswitch?powerlow:power,
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes4, len(bytes4));
          break;
        }

        case Qt::Key_Left : {
          byte bytes3[] = { 0x04, 0x00, lowswitch?non(powerlow):non(power),
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes3, len(bytes3));

//          byte bytes4[] = { 0x04, 0x02, lowswitch?powerlow:power,
//                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
//          net->directCommand(bytes4, len(bytes4));
          break;
        }

        case Qt::Key_Right : {
          byte bytes3[] = { 0x04, 0x00, lowswitch?powerlow:power,
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes3, len(bytes3));

//          byte bytes4[] = { 0x04, 0x02, lowswitch?non(powerlow):non(power),
//                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
//          net->directCommand(bytes4, len(bytes4));
          break;
        }

        case Qt::Key_N : {
          byte bytes3[] = { 0x04, 0x00, lowswitch?powerlow:power,
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes3, len(bytes3));
          break;
        }

        case Qt::Key_M : {
          byte bytes3[] = { 0x04, 0x00, lowswitch?non(powerlow):non(power),
                            0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00 };
          net->directCommand(bytes3, len(bytes3));
          break;
        }

        case Qt::Key_Minus : {
          if (lowswitch) {
            if (powerlow>0x32) powerlow--;
            lowspeed->setValue(powerlow);
          }
          else {
            if (power>0x32) power--;
            highspeed->setValue(power);
          }
          break;
        }

        case Qt::Key_Plus : {
          if (lowswitch) {
            if (powerlow<0x64) powerlow++;
            lowspeed->setValue(powerlow);
          }
          else {
            if (power<0x64) power++;
            highspeed->setValue(power);
          }
          break;
        }

      }
    }
    else {
      switch (event->key()) {
        case Qt::Key_Minus : {
          if (lowswitch) {
            if (powerlow>0x32) powerlow--;
            lowspeed->setValue(powerlow);
          }
          else
          {
            if (power>0x32) power--;
            highspeed->setValue(power);
          }
          break;
        }

        case Qt::Key_Plus : {
          if (lowswitch) {
            if (powerlow<0x64) powerlow++;
            lowspeed->setValue(powerlow);
          }
          else {
            if (power<0x64) power++;
            highspeed->setValue(power);
          }
          break;
        }
      }
    }
  }

  /** ----------------------------------------------------------------------
   * @brief keyReleaseEvent leave commands when user is play with NXT PC
   * Remote Control.
   */
  void keyReleaseEvent(QKeyEvent *event) {
    if (bind->text() == idiom.getConnectButtonLabel()) return;
    if (!event->isAutoRepeat()) {
      switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_N:
        case Qt::Key_M: {
          byte bytes0[] = { 0x04, 0x00, power, 0x02, 0x01, 0x00, 0x20, 0x00,
                            0x00, 0x00, 0x00 };
          net->directCommand(bytes0, len(bytes0));

          byte bytes1[] = { 0x04, 0x01, power, 0x02, 0x01, 0x00, 0x20, 0x00,
                            0x00, 0x00, 0x00 };
          net->directCommand(bytes1, len(bytes1));

          byte bytes2[] = { 0x04, 0x02, power, 0x02, 0x01, 0x00, 0x20, 0x00,
                            0x00, 0x00, 0x00 };
          net->directCommand(bytes2, len(bytes2));
          break;
        }

        case Qt::Key_Alt: {
          lowswitch = false;
          break;
        }

      }
    }
  }

public slots:

  /** ----------------------------------------------------------------------
   * @brief scanDevices method search all devices with bluetooth
   * functionality.
   */
  void scanDevices() {
    devices->clear();
    devices->setEnabled(false);
    bind->setEnabled(false);
    scan->setEnabled(false);
    devices->addItem(idiom.getMessageSearching());
    info->setPixmap(QPixmap(":/images/clock.png"));
    info->setEnabled(false);

    t = new Thread(devices,net,1);
    connect(t,SIGNAL(scanPerformed(int)),this,SLOT(scanPerformed(int)));
    t->start();
  }

  /** ----------------------------------------------------------------------
   * @brief scanPerformed is run after net scan->
   */
  void scanPerformed(int throwstate) {
    info->setPixmap(QPixmap(idiom.getImageInfo()));
    info->setEnabled(true);
    devices->setEnabled(true);
    scan->setEnabled(true);
    info->setPixmap(QPixmap(idiom.getImageInfo()));
    if (throwstate==0) {
      bind->setEnabled(true);
    }
    else {
      bind->setEnabled(false);
      switch(throwstate) {
        case 1: {
          devices->addItem(idiom.getMessageBluetoothDisabled());
          break;
        }
        case 2: {
          devices->addItem(idiom.getMessageNearDivices());
          break;
        }
      }
    }
    delete t;
  }

  /** ----------------------------------------------------------------------
   * @brief connectDevice bind NXT PC Remote Control with a wanted device
   */
  void connectDevice() {
    if (bind->text() == idiom.getConnectButtonLabel()) {
      info->setPixmap(QPixmap(":/images/clock.png"));
      info->setEnabled(false);
      bind->setEnabled(false);
      scan->setEnabled(false);
      devices->setEnabled(false);
      t = new Thread(devices,net,2);
      connect(t,SIGNAL(connectPerformed(bool)),this,SLOT(connectPerformed(bool)));
      t->start();
    }
    else {
      net->unbind();
      scan->setEnabled(true);
      devices->setEnabled(true);
      bind->setText(idiom.getConnectButtonLabel());
      recents->setEnabled(true);
      for (int i=0; i<4;i++) menu->actions().at(i)->setEnabled(true);
    }
  }

  /** ----------------------------------------------------------------------
   * @brief connectPerfomred is run after net bind function.
   */
  void connectPerformed(bool ok) {
    info->setPixmap(QPixmap(idiom.getImageInfo()));
    info->setEnabled(true);
    if (ok) {
      bind->setText(idiom.getDisconnectButtonLabel());
      addRecent(devices->currentText());
      for (int i=0; i<4;i++) menu->actions().at(i)->setEnabled(false);
    }
    else {
      scan->setEnabled(true);
      devices->clear();
      devices->addItem(idiom.getMessageDeviceAvailable());
      devices->setEnabled(true);
    }
    bind->setEnabled(true);
    delete t;
  }

  /** ----------------------------------------------------------------------
   * @brief popMenu method show a flotating menu with some additional
   * options.
   */
  void popMenu(QPoint point) {
    menu->exec(info->mapToGlobal(point));
  }

  /** ----------------------------------------------------------------------
   * @brief recentSelection method short way, in the connection event.  It
   * method is exec when an user use the cache connections.
   */
  void recentSelection(QAction* action) {
    info->setPixmap(QPixmap(":/images/clock.png"));
    info->setEnabled(false);
    bind->setEnabled(false);
    scan->setEnabled(false);
    devices->clear();
    devices->addItem(action->text().left(action->text().size()-1));
    devices->setEnabled(false);
    t = new Thread(devices,net,2);
    connect(t,SIGNAL(connectPerformed(bool)),this,SLOT(connectPerformed(bool)));
    t->start();
  }

  /** ----------------------------------------------------------------------
   * @brief changeIdiom method switch de interface language between Englis
   * and Spanish
   */
  void changeIdiom(QAction *action) {
     if (action->text().left(3)==idiom.getMenuEnglish().left(3) && idiom.getIdiomType()!=ENG) {
       idiom.setIdiomType(ENG);
       refreshIdiom();
     }
     else if (action->text().left(3)==idiom.getMenuSpanish().left(3) &&
              idiom.getIdiomType()!=SPA) {
       idiom.setIdiomType(SPA);
       refreshIdiom();
     }
  }

  /** ----------------------------------------------------------------------
   * @brief menuOption method exec two of all additional options: Show about,
   * and Clear cache connections
   */
  void menuOption(QAction* action) {
    if (action->text()==idiom.getMenuAbout()) {
      showAbout(true);
    }
    else if (action->text()==idiom.getMenuClearConnections()) {
      recents->clear();
    }
  }

  /** ----------------------------------------------------------------------
   * @brief showAbout method, show the author information.
   */
  void showAbout(bool state) {
    if (state==true) {
      info->setPixmap(QPixmap(":/images/about.png"));
    }
    else {
      info->setPixmap(QPixmap(idiom.getImageInfo()));
    }
  }

};

#endif // WINDOW_H
