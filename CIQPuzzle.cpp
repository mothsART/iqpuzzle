/**
 * \file CIQPuzzle.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2012 Thorsten Roth <elthoro@gmx.de>
 *
 * This file is part of iQPuzzle.
 *
 * iQPuzzle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iQPuzzle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iQPuzzle.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CIQPuzzle.h"
#include "ui_CIQPuzzle.h"

CIQPuzzle::CIQPuzzle(  QApplication *pApp, QWidget *pParent ) :
    QMainWindow( pParent ),
    m_pUi( new Ui::CIQPuzzle ),
    m_pApp( pApp )
{
    qDebug() << "Enter" << Q_FUNC_INFO;

    // Init config file
    QSettings::setPath( QSettings::NativeFormat, QSettings::UserScope,
                        m_pApp->applicationDirPath() );
    m_pConfig = new QSettings( QSettings::NativeFormat, QSettings::UserScope,
                               m_pApp->applicationName() );

    m_nGridSize = m_pConfig->value("GridSize", 25 ).toUInt();
    m_BoardSize.setWidth( m_pConfig->value("BoardWidth", 8 ).toUInt() );
    m_BoardSize.setHeight( m_pConfig->value("BoardHeight", 8 ).toUInt() );

    m_pUi->setupUi(this);

    this->setupMenu();

    // Create board
    m_pGraphView = new QGraphicsView( this );
    this->setCentralWidget( m_pGraphView );
    m_pScene = new QGraphicsScene( this );
    m_pGraphView->setScene( m_pScene );
    this->setupBoard();

    this->startNewGame();

    qDebug() << "Leave" << Q_FUNC_INFO;
}

CIQPuzzle::~CIQPuzzle()
{
    if ( m_pUi != NULL )
        delete m_pUi;
    m_pUi = NULL;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CIQPuzzle::setupMenu()
{
    qDebug() << "Enter" << Q_FUNC_INFO;

    // New game
    m_pUi->action_NewGame->setShortcut( QKeySequence::New );
    connect( m_pUi->action_NewGame, SIGNAL(triggered()),
             this, SLOT(startNewGame()) );

    // Exit game
    m_pUi->action_Quit->setShortcut( QKeySequence::Quit );
    connect( m_pUi->action_Quit, SIGNAL(triggered()),
             this, SLOT(close()) );

    // Zoom in/out
    m_pUi->action_ZoomIn->setShortcut( QKeySequence::ZoomIn );
    connect( m_pUi->action_ZoomIn, SIGNAL(triggered()),
             this, SLOT(zoomIn()) );
    m_pUi->action_ZoomOut->setShortcut( QKeySequence::ZoomOut );
    connect( m_pUi->action_ZoomOut, SIGNAL(triggered()),
             this, SLOT(zoomOut()) );

    // Controls info
    m_pUi->action_Controls->setShortcut( QKeySequence::HelpContents );
    connect( m_pUi->action_Controls, SIGNAL(triggered()),
             this, SLOT(showControlsBox()) );

    // About
    connect( m_pUi->action_Info, SIGNAL(triggered()),
             this, SLOT(showInfoBox()) );

    qDebug() << "Leave" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CIQPuzzle::setupBoard()
{
    qDebug() << "Enter" << Q_FUNC_INFO;

    // Set main window (fixed) size
    const QSize WinSize( m_BoardSize.width() * m_nGridSize + 10 * m_nGridSize,
                         m_BoardSize.height() * m_nGridSize + 10 * m_nGridSize);

    this->setMinimumSize( WinSize );
    // this->setMaximumSize( WinSize );

    QRectF rectBoard( QPoint(0,0), QSizeF(m_BoardSize.width()*m_nGridSize,
                                          m_BoardSize.height()*m_nGridSize) );
    m_pScene->addRect( rectBoard );
    m_pGraphView->setSceneRect( rectBoard );

    // Draw grid
    QLineF lineGrid;
    QPen gridPen( QColor(220, 220, 220) );
    // Horizontal
    for( int i = 1; i < m_BoardSize.height(); i++ )
    {
        lineGrid.setLine(1, i*m_nGridSize, m_BoardSize.width()*m_nGridSize-1, i*m_nGridSize);
        m_pScene->addLine( lineGrid, gridPen );
    }
    // Vertical
    for( int i = 1; i < m_BoardSize.width(); i++ )
    {
        lineGrid.setLine(i*m_nGridSize, 1, i*m_nGridSize, m_BoardSize.height()*m_nGridSize-1);
        m_pScene->addLine(lineGrid, gridPen );
    }

    qDebug() << "Leave" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CIQPuzzle::setupBlocks()
{
    qDebug() << "Enter" << Q_FUNC_INFO;

    QPolygonF tmpPolygon;
    QColor tmpColor;
    QPointF tmpPoint;

    QString sTmpValue;
    QStringList sListVector;
    QStringList sListPoint;

    // Get number of blocks
    unsigned int nNumOfBlocks = m_pConfig->value("NumOfBlocks", 0 ).toUInt();

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    unsigned int nStartBlock = 0;
    if( nNumOfBlocks != 0 )
    {
        nStartBlock = qrand() % ((nNumOfBlocks + 1) - 1) + 1;
    }

    m_listBlocks.clear();
    // Get block properties
    for( unsigned int i = 1; i <= nNumOfBlocks; i++ )
    {
        m_pConfig->beginGroup( "Block" + QString::number(i) );
        sTmpValue = m_pConfig->value("Polygon", "").toString();

        // Get polygon
        sListVector.clear();
        sListVector << sTmpValue.split("|");
        tmpPolygon.clear();
        foreach( QString s, sListVector)
        {
            sListPoint.clear();
            sListPoint << s.split(",");
            if( sListPoint.size() >= 2 )
            {
                tmpPolygon << QPointF( sListPoint[0].trimmed().toShort(),
                                       sListPoint[1].trimmed().toShort() );
            }
        }

        // Get color
        sTmpValue = m_pConfig->value("Color", "0,0,0").toString();
        if( sTmpValue.count(',') < 2 )
            sTmpValue = "200,200,200";
        sListVector.clear();
        sListVector << sTmpValue.split(",");
        if( sListVector.size() >= 3 )
        {
            tmpColor.setRgb( sListVector[0].trimmed().toShort(),
                             sListVector[1].trimmed().toShort(),
                             sListVector[2].trimmed().toShort() );
        }

        // Get start position
        if ( i != nStartBlock )
        {
            sTmpValue = m_pConfig->value("StartPos", "-4,-4").toString();
            if( sTmpValue.count(',') != 1 )
                sTmpValue = "-4,-4";
            sListVector.clear();
            sListVector << sTmpValue.split(",");
            if( sListVector.size() >= 2 )
            {
                tmpPoint.setX( sListVector[0].trimmed().toInt() );
                tmpPoint.setY( sListVector[1].trimmed().toInt() );
            }
        }
        // This block is random start block
        else
        {
            tmpPoint.setX( 0 );
            tmpPoint.setY( 0 );

            /* ToDo:
             * Set random rotation
             */
        }

        // Create new block
        m_listBlocks.append( new CBlock(tmpPolygon, m_nGridSize, tmpColor, i, &m_listBlocks, tmpPoint) );
        m_pConfig->endGroup();
    }

    qDebug() << "Leave" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CIQPuzzle::startNewGame()
{
    // Delete existing all existing QGraphicItems in scene
    QList<QGraphicsItem *> list = m_pScene->items();
    qDeleteAll( list );
    m_listBlocks.clear();

    // Create board and blocks
    this->setupBoard();
    this->setupBlocks();

    // Insert blocks on board again
    foreach( CBlock *pB, m_listBlocks )
        m_pScene->addItem( pB );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CIQPuzzle::zoomIn()
{
    m_nGridSize += 5;
    doZoom();
}

void CIQPuzzle::zoomOut()
{
    if( m_nGridSize > 9 )
        m_nGridSize -= 5;
    else
        m_nGridSize = 5;

    doZoom();
}

void CIQPuzzle::doZoom()
{
    // Get all QGraphicItems in scene
    QList<QGraphicsItem *> list = m_pScene->items();
    QList<QGraphicsItem *> list2;

    // Filter out blocks from object list
    foreach(QGraphicsItem* gi, list)
    {
        if( gi->type() != CBlock::Type )
            list2.append(gi);
    }
    // Remove everything except blocks
    qDeleteAll( list2 );

    // Rescale blocks
    foreach( CBlock *pB, m_listBlocks )
    {
        pB->rescaleBlock( m_nGridSize );
    }

    // Create board again
    this->setupBoard();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CIQPuzzle::showControlsBox()
{
    QDialog dialog(this);
    dialog.setWindowTitle( tr("Controls") );

    QGridLayout* layout = new QGridLayout(&dialog);
    layout->setMargin(12);
    layout->setSpacing(0);
    layout->setColumnMinimumWidth(1, 2);
    layout->setRowMinimumHeight(3, 12);

    layout->addWidget( new QLabel(tr("<b>Move block:</b>"), &dialog),
                       0, 0, Qt::AlignRight | Qt::AlignVCenter );
    layout->addWidget( new QLabel(tr("Drag & drop with left mouse button"), &dialog),
                       0, 2, Qt::AlignLeft | Qt::AlignVCenter );
    layout->addWidget( new QLabel(tr("<b>Rotate block:</b>"), &dialog),
                       1, 0, Qt::AlignRight | Qt::AlignVCenter );
    layout->addWidget( new QLabel(tr("Mousewheel up/down on block"), &dialog),
                       1, 2, Qt::AlignLeft | Qt::AlignVCenter );
    layout->addWidget( new QLabel(tr("<b>Flip block:</b>"), &dialog),
                       2, 0, Qt::AlignRight | Qt::AlignVCenter );
    layout->addWidget( new QLabel(tr("Right click on block"), &dialog),
                       2, 2, Qt::AlignLeft | Qt::AlignVCenter );

    QDialogButtonBox* buttons = new QDialogButtonBox( QDialogButtonBox::Close,
                                                      Qt::Horizontal, &dialog );
    connect( buttons, SIGNAL(rejected()),
             &dialog, SLOT(reject()) );
    layout->addWidget( buttons, 4, 0, 1, 3 );

    dialog.exec();
}

// -----------------------------------------------------------------------------------------------

void CIQPuzzle::showInfoBox()
{
    QMessageBox::about( this, tr("About"),
                        tr("<center><big><b>%1 %2</b></big><br/>"
                           "A diverting I.Q. challenging puzzle<br/>"
                           "<small>Copyright &copy; 2012 Thorsten Roth</small></center>")
                        .arg(m_pApp->applicationName())
                        .arg(m_pApp->applicationVersion()) );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Close event (File -> Close or X)
void CIQPuzzle::closeEvent( QCloseEvent *pEvent )
{
    pEvent->accept();

    /*
    int nRet = QMessageBox::question( this, tr("Quit") + " - " + m_pApp->applicationName(),
                                     tr("Do you really want to quit?"),
                                     QMessageBox::Yes | QMessageBox::No );

    if ( QMessageBox::Yes == nRet )
    {
        pEvent->accept();
    }
    else
    {
        pEvent->ignore();
    }
    */
}