/************************************************************************
 **
 **  @file   vsimplepoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vsimplepoint.h"
#include "vgraphicssimpletextitem.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QPen>

//---------------------------------------------------------------------------------------------------------------------
VSimplePoint::VSimplePoint(quint32 id, const QColor &currentColor, Unit patternUnit, qreal *factor, QObject *parent)
    :VAbstractSimple(id, currentColor, patternUnit, factor, parent), QGraphicsEllipseItem(),
      radius(ToPixel(DefPointRadius/*mm*/, Unit::Mm)), namePoint(nullptr), lineName(nullptr)
{
    namePoint = new VGraphicsSimpleTextItem(this);
    connect(namePoint, &VGraphicsSimpleTextItem::ShowContextMenu, this, &VSimplePoint::ContextMenu);
    connect(namePoint, &VGraphicsSimpleTextItem::DeleteTool, this, &VSimplePoint::DeleteFromLabel);
    connect(namePoint, &VGraphicsSimpleTextItem::PointChoosed, this, &VSimplePoint::PointChoosed);
    connect(namePoint, &VGraphicsSimpleTextItem::NameChangePosition, this, &VSimplePoint::ChangedPosition);
    lineName = new QGraphicsLineItem(this);
    this->setBrush(QBrush(Qt::NoBrush));
    SetPen(this, currentColor, WidthHairLine(patternUnit));
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);
    this->setAcceptHoverEvents(true);
}

//---------------------------------------------------------------------------------------------------------------------
VSimplePoint::~VSimplePoint()
{}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::ChangedActivDraw(const bool &flag)
{
    enabled = flag;
    setEnabled(enabled);
    SetPen(this, currentColor, WidthHairLine(patternUnit));
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    /* From question on StackOverflow
     * https://stackoverflow.com/questions/10985028/how-to-remove-border-around-qgraphicsitem-when-selected
     *
     * There's no interface to disable the drawing of the selection border for the build-in QGraphicsItems. The only way
     * I can think of is derive your own items from the build-in ones and override the paint() function:*/
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsEllipseItem::paint(painter, &myOption, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::RefreshLine()
{
    QRectF nRec = namePoint->sceneBoundingRect();
    nRec.translate(- scenePos());
    if (this->rect().intersects(nRec) == false)
    {
        const QRectF nameRec = namePoint->sceneBoundingRect();
        QPointF p1, p2;
        VGObject::LineIntersectCircle(QPointF(), radius, QLineF(QPointF(), nameRec.center() - scenePos()), p1, p2);
        const QPointF pRec = VGObject::LineIntersectRect(nameRec, QLineF(scenePos(), nameRec.center()));
        lineName->setLine(QLineF(p1, pRec - scenePos()));
        SetPen(lineName, QColor(Qt::black), WidthHairLine(patternUnit));

        if (QLineF(p1, pRec - scenePos()).length() <= ToPixel(4, Unit::Mm))
        {
            lineName->setVisible(false);
        }
        else
        {
            lineName->setVisible(true);
        }
    }
    else
    {
        lineName->setVisible(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::RefreshGeometry(const VPointF &point)
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    SetPen(this, currentColor, WidthHairLine(patternUnit));
    QRectF rec = QRectF(0, 0, radius*2, radius*2);
    rec.translate(-rec.center().x(), -rec.center().y());
    this->setRect(rec);
    this->setPos(point.toQPointF());
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    namePoint->blockSignals(true);
    QFont font = namePoint->font();
    if (factor == nullptr)
    {
        font.setPointSize(static_cast<qint32>(namePoint->FontSize()));
    }
    else
    {
        font.setPointSize(static_cast<qint32>(namePoint->FontSize()/ *factor));
    }
    namePoint->setFont(font);
    namePoint->setText(point.name());
    namePoint->setPos(QPointF(point.mx(), point.my()));
    namePoint->blockSignals(false);
    RefreshLine();
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::SetEnabled(bool enabled)
{
    namePoint->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::EnableToolMove(bool move)
{
    namePoint->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::DeleteFromLabel()
{
    emit Delete();
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::PointChoosed()
{
    emit Choosed(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::ChangedPosition(const QPointF &pos)
{
    emit NameChangedPosition(pos);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::ContextMenu(QGraphicsSceneContextMenuEvent *event)
{
    emit ShowContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit Choosed(id);
    }
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    SetPen(this, currentColor, WidthMainLine(patternUnit));
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    SetPen(this, currentColor, WidthHairLine(patternUnit));
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
QColor VSimplePoint::GetCurrentColor() const
{
    return currentColor;
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::SetCurrentColor(const QColor &value)
{
    currentColor = value;
    SetPen(this, CorrectColor(currentColor), pen().widthF());
}