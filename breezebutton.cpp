/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "breezebutton.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
//#include <KIconLoader>

#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

namespace Breeze
{
    using KDecoration3::ColorGroup;
    using KDecoration3::ColorRole;
    using KDecoration3::DecorationButtonType;


    //__________________________________________________________________
    Button::Button(DecorationButtonType type, Decoration* decoration, QObject* parent)
        : DecorationButton(type, decoration, parent)
        , m_animation(new QVariantAnimation(this))
    {

        // setup animation
        // It is important start and end value are of the same type, hence 0.0 and not just 0
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });

        // connections
        connect(decoration->window(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().get(), &KDecoration3::DecorationSettings::reconfigured, this, &Button::reconfigure);
        connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, &Button::updateAnimationState);

        reconfigure();

    }

    //__________________________________________________________________
    Button::Button(QObject *parent, const QVariantList &args)
        : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
    {
        setGeometry(QRectF(QPointF(0, 0), preferredSize()));
    }

    //__________________________________________________________________
    Button *Button::create(DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
    {
        if (auto d = qobject_cast<Decoration*>(decoration))
        {
            Button *b = new Button(type, d, parent);
            const auto c = d->window();
            switch (type)
            {

                case DecorationButtonType::Close:
                b->setVisible(c->isCloseable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::closeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Maximize:
                b->setVisible(c->isMaximizeable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Minimize:
                b->setVisible(c->isMinimizeable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::ContextHelp:
                b->setVisible(c->providesContextHelp());
                QObject::connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Shade:
                b->setVisible(c->isShadeable());
                QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &Breeze::Button::setVisible);
                break;

                case DecorationButtonType::Menu:
                QObject::connect(c, &KDecoration3::DecoratedWindow::iconChanged, b, [b]() { b->update(); });
                break;

                default: break;

            }

            return b;
        }

        return nullptr;

    }

    //__________________________________________________________________
    void Button::paint(QPainter *painter, const QRectF &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (!decoration()) return;

        painter->save();

        // menu button
        if (type() == DecorationButtonType::Menu)
        {
            const QRectF iconRect = geometry().marginsRemoved(m_padding);
            const auto w = decoration()->window();
            /*if (auto deco = qobject_cast<Decoration *>(decoration())) {
                const QPalette activePalette = KIconLoader::global()->customPalette();
                QPalette palette = w->palette();
                palette.setColor(QPalette::WindowText, deco->fontColor());
                KIconLoader::global()->setCustomPalette(palette);
                w->icon().paint(painter, iconRect.toRect());
                if (activePalette == QPalette()) {
                    KIconLoader::global()->resetPalette();
                } else {
                    KIconLoader::global()->setCustomPalette(palette);
                }
            } else {*/
                w->icon().paint(painter, iconRect.toRect());
            //}
        }
    } else {

        auto d = qobject_cast<Decoration*>( decoration() );

        if ( d && d->internalSettings()->buttonStyle() == 0 )
            drawIconPlasma( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 1 )
            drawIconGnome( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 2 )
            drawIconMacSierra( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 3 )
            drawIconMacDarkAurorae( painter );
        else if ( d && ( d->internalSettings()->buttonStyle() == 4 || d->internalSettings()->buttonStyle() == 5 || d->internalSettings()->buttonStyle() == 6 ) )
            drawIconSBEsierra( painter );
        else if ( d && ( d->internalSettings()->buttonStyle() == 7 || d->internalSettings()->buttonStyle() == 8 || d->internalSettings()->buttonStyle() == 9 ) )
            drawIconSBEdarkAurorae( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 10 )
            drawIconSierraColorSymbols( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 11 )
            drawIconDarkAuroraeColorSymbols( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 12 )
            drawIconSierraMonochromeSymbols( painter );
        else if ( d && d->internalSettings()->buttonStyle() == 13 )
            drawIconDarkAuroraeMonochromeSymbols( painter );

    }
        painter->restore();

    }

    //__________________________________________________________________
    void Button::drawIcon(QPainter *painter) const
    {

        painter->setRenderHints(QPainter::Antialiasing);

        /*
        scale painter so that its window matches QRect(-1, -1, 20, 20)
        this makes all further rendering and scaling simpler
        all further rendering is performed inside QRect(0, 0, 18, 18)
        */
        const QRectF rect = geometry().marginsRemoved(m_padding);
        painter->translate(rect.topLeft());

        const qreal width(rect.width());
        painter->scale(width/20, width/20);
        painter->translate(1, 1);

        // render background
        const QColor backgroundColor(this->backgroundColor());

        auto d = qobject_cast<Decoration*>(decoration());
        bool isInactive(d && !d->window()->isActive()
                        && !isHovered() && !isPressed()
                        && m_animation->state() != QAbstractAnimation::Running);
        QColor inactiveCol(Qt::gray);
        if (isInactive)
        {
            int gray = qGray(d->titleBarColor().rgb());
            if (gray <= 200) {
                gray += 55;
                gray = qMax(gray, 115);
            }
            else gray -= 45;
            inactiveCol = QColor(gray, gray, gray);
        }

        // render mark
        const QColor foregroundColor(this->foregroundColor(inactiveCol));
        if (foregroundColor.isValid())
        {

            // setup painter
            QPen pen(foregroundColor);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, 20/width));

            switch (type())
            {

                case DecorationButtonType::Close:
                {
                    if (!d || d->internalSettings()->macOSButtons()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(255, 92, 87));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(233, 84, 79));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(250, 100, 102));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(230, 92, 94));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + (isPressed() ? 0.0
                                         : static_cast<qreal>(2) * m_animation->currentValue().toReal());
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    else {
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawLine(QPointF(5, 5), QPointF(13, 13));
                        painter->drawLine(QPointF(5, 13), QPointF(13, 5));
                    }
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    if (!d || d->internalSettings()->macOSButtons()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        {
                            grad.setColorAt(0, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(67, 198, 176)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(40, 211, 63));
                            grad.setColorAt(1, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(60, 178, 159)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(36, 191, 57));
                        }
                        else
                        {
                            grad.setColorAt(0, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(67, 198, 176)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(124, 198, 67));
                            grad.setColorAt(1, isChecked() ? isInactive ? inactiveCol
                                                                        : QColor(60, 178, 159)
                                                           : isInactive ? inactiveCol
                                                                        : QColor(111, 178, 60));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + (isPressed() ? 0.0
                                         : static_cast<qreal>(2) * m_animation->currentValue().toReal());
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    else {
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }

                        if (isHovered())
                            pen.setWidthF(1.2*qMax((qreal)1.0, 20/width));
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawPolyline(QPolygonF()
                                                << QPointF(5, 8) << QPointF(5, 13) << QPointF(10, 13));
                        if (isChecked())
                            painter->drawRect(QRectF(8.0, 5.0, 5.0, 5.0));
                        else {
                            painter->drawPolyline(QPolygonF()
                                                  << QPointF(8, 5) << QPointF(13, 5) << QPointF(13, 10));
                        }

                        if (isHovered())
                            pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, 20/width));
                    }
                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    if (!d || d->internalSettings()->macOSButtons()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(243, 176, 43));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(223, 162, 39));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(237, 198, 81));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(217, 181, 74));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + (isPressed() ? 0.0
                                         : static_cast<qreal>(2) * m_animation->currentValue().toReal());
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    else {
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }

                        if (isHovered())
                            pen.setWidthF(1.2*qMax((qreal)1.0, 20/width));
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawLine(QPointF(4, 9), QPointF(14, 9));

                        if (isHovered())
                            pen.setWidthF(PenWidth::Symbol*qMax((qreal)1.0, 20/width));
                    }
                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        painter->setPen(Qt::NoPen);
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setBrush(foregroundColor);

                        if (macOSBtn)
                            painter->drawEllipse(QRectF(6, 6, 6, 6));
                        else {
                            if (isChecked()) {

                                // outer ring
                                painter->drawEllipse(QRectF(3, 3, 12, 12));

                                // center dot
                                QColor backgroundColor(this->backgroundColor());
                                if (!backgroundColor.isValid() && d) backgroundColor = d->titleBarColor();

                                if (backgroundColor.isValid())
                                {
                                    painter->setBrush(backgroundColor);
                                    painter->drawEllipse(QRectF(8, 8, 2, 2));
                                }

                            }
                            else {
                                painter->drawPolygon(QPolygonF()
                                    << QPointF(6.5, 8.5)
                                    << QPointF(12, 3)
                                    << QPointF(15, 6)
                                    << QPointF(9.5, 11.5));

                                painter->setPen(pen);
                                painter->drawLine(QPointF(5.5, 7.5), QPointF(10.5, 12.5));
                                painter->drawLine(QPointF(12, 6), QPointF(4.5, 13.5));
                            }
                        }
                    }
                    break;
                }

                case DecorationButtonType::Shade:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        painter->drawLine(5, 6, 13, 6);
                        if (isChecked()) {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 9)
                                << QPointF(9, 13)
                                << QPointF(13, 9));

                        }
                        else {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 13)
                                << QPointF(9, 9)
                                << QPointF(13, 13));
                        }
                    }

                    break;

                }

                case DecorationButtonType::KeepBelow:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons() || isChecked());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        if (macOSBtn) {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 6)
                                << QPointF(9, 9)
                                << QPointF(12, 6));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 10)
                                << QPointF(9, 13)
                                << QPointF(12, 10));
                        }
                        else {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 5)
                                << QPointF(9, 9)
                                << QPointF(13, 5));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 9)
                                << QPointF(9, 13)
                                << QPointF(13, 9));
                        }
                    }
                    break;

                }

                case DecorationButtonType::KeepAbove:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        if (isChecked())
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        else {
                            painter->drawEllipse(QRectF(2, 2, 14, 14));
                            if (backgroundColor.isValid())
                            {
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(backgroundColor);
                                qreal r = static_cast<qreal>(7)
                                          + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                                painter->drawEllipse(c, r, r);
                            }
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered() || isChecked()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        if (macOSBtn) {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 8)
                                << QPointF(9, 5)
                                << QPointF(12, 8));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(6, 12)
                                << QPointF(9, 9)
                                << QPointF(12, 12));
                        }
                        else {
                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 9)
                                << QPointF(9, 5)
                                << QPointF(13, 9));

                            painter->drawPolyline(QPolygonF()
                                << QPointF(5, 13)
                                << QPointF(9, 9)
                                << QPointF(13, 13));
                        }
                    }
                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(230, 129, 67));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(210, 118, 61));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(250, 145, 100));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(230, 131, 92));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        if (macOSBtn) {
                            painter->drawLine(QPointF(4.5, 6), QPointF(13.5, 6));
                            painter->drawLine(QPointF(4.5, 9), QPointF(13.5, 9));
                            painter->drawLine(QPointF(4.5, 12), QPointF(13.5, 12));
                        }
                        else {
                            painter->drawLine(QPointF(3.5, 5), QPointF(14.5, 5));
                            painter->drawLine(QPointF(3.5, 9), QPointF(14.5, 9));
                            painter->drawLine(QPointF(3.5, 13), QPointF(14.5, 13));
                        }
                    }
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                    bool macOSBtn(!d || d->internalSettings()->macOSButtons());
                    if (macOSBtn && !isPressed()) {
                        QLinearGradient grad(QPointF(9, 2), QPointF(9, 16));
                        if (d && qGray(d->titleBarColor().rgb()) > 100)
                        { // yellow isn't good with light backgrounds
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(103, 149, 210));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(93, 135, 190));
                        }
                        else
                        {
                            grad.setColorAt(0, isInactive ? inactiveCol
                                                          : QColor(135, 166, 220));
                            grad.setColorAt(1, isInactive ? inactiveCol
                                                          : QColor(122, 151, 200));
                        }
                        painter->setBrush(QBrush(grad));
                        painter->setPen(Qt::NoPen);
                        painter->drawEllipse(QRectF(2, 2, 14, 14));
                        if (backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            qreal r = static_cast<qreal>(7)
                                      + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                            QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                            painter->drawEllipse(c, r, r);
                        }
                    }
                    if (!macOSBtn || isPressed() || isHovered()) {
                        if ((!macOSBtn  || isPressed()) && backgroundColor.isValid())
                        {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(backgroundColor);
                            painter->drawEllipse(QRectF(0, 0, 18, 18));
                        }
                        painter->setPen(pen);
                        painter->setBrush(Qt::NoBrush);

                        QPainterPath path;
                        path.moveTo(5, 6);
                        path.arcTo(QRectF(5, 3.5, 8, 5), 180, -180);
                        path.cubicTo(QPointF(12.5, 9.5), QPointF(9, 7.5), QPointF(9, 11.5));
                        painter->drawPath(path);

                        painter->drawPoint(9, 15);
                    }

                    break;
                }

                default: break;

            }

        }

    }

    //__________________________________________________________________
    void Button::drawIconPlasma( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        // render background
        QColor backgroundColor( this->backgroundColor() );
        if( backgroundColor.isValid() )
        {
            painter->setPen( Qt::NoPen );
            painter->setBrush( backgroundColor );
            painter->drawEllipse( QRectF( 0, 0, 18, 18 ) );
        }

        // render mark
        QColor foregroundColor( this->foregroundColor() );
        if( foregroundColor.isValid() )
        {

            // setup painter
            QPen pen( foregroundColor );
            pen.setCapStyle( Qt::RoundCap );
            pen.setJoinStyle( Qt::MiterJoin );
            pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

            painter->setPen( pen );
            painter->setBrush( Qt::NoBrush );

            switch( type() )
            {

                case DecorationButtonType::Close:
                {
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                    painter->drawLine( 13, 5, 5, 13 );
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    if( isChecked() )
                    {
                        pen.setJoinStyle( Qt::RoundJoin );
                        painter->setPen( pen );

                        painter->drawPolygon( QVector<QPointF>{
                            QPointF( 4, 9 ),
                                              QPointF( 9, 4 ),
                                              QPointF( 14, 9 ),
                                              QPointF( 9, 14 )} );

                    } else {
                        painter->drawPolyline( QVector<QPointF>{
                            QPointF( 4, 11 ),
                                               QPointF( 9, 6 ),
                                               QPointF( 14, 11 )});
                    }
                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    painter->drawPolyline( QVector<QPointF>{
                        QPointF( 4, 7 ),
                                           QPointF( 9, 12 ),
                                           QPointF( 14, 7 ) });
                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {
                    painter->setPen( Qt::NoPen );
                    painter->setBrush( foregroundColor );

                    if( isChecked())
                    {

                        // outer ring
                        painter->drawEllipse( QRectF( 3, 3, 12, 12 ) );

                        // center dot
                        QColor backgroundColor( this->backgroundColor() );
                        auto d = qobject_cast<Decoration*>( decoration() );
                        if( !backgroundColor.isValid() && d ) backgroundColor = d->titleBarColor();

                        if( backgroundColor.isValid() )
                        {
                            painter->setBrush( backgroundColor );
                            painter->drawEllipse( QRectF( 8, 8, 2, 2 ) );
                        }

                    } else {

                        painter->drawPolygon( QVector<QPointF> {
                            QPointF( 6.5, 8.5 ),
                                              QPointF( 12, 3 ),
                                              QPointF( 15, 6 ),
                                              QPointF( 9.5, 11.5 )} );

                        painter->setPen( pen );
                        painter->drawLine( QPointF( 5.5, 7.5 ), QPointF( 10.5, 12.5 ) );
                        painter->drawLine( QPointF( 12, 6 ), QPointF( 4.5, 13.5 ) );
                    }
                    break;
                }

                case DecorationButtonType::Shade:
                {

                    if (isChecked())
                    {

                        painter->drawLine( QPointF( 4, 5.5 ), QPointF( 14, 5.5 ) ); // painter->drawLine( 4, 5, 14, 5 );
                        painter->drawPolyline( QVector<QPointF> {
                            QPointF( 4, 8 ),
                                               QPointF( 9, 13 ),
                                               QPointF( 14, 8 )} );

                    } else {

                        painter->drawLine( QPointF( 4, 5.5 ), QPointF( 14, 5.5 ) ); // painter->drawLine( 4, 5, 14, 5 );
                        painter->drawPolyline(  QVector<QPointF> {
                            QPointF( 4, 13 ),
                                                QPointF( 9, 8 ),
                                                QPointF( 14, 13 ) });
                    }

                    break;

                }

                case DecorationButtonType::KeepBelow:
                {

                    painter->drawPolyline(  QVector<QPointF> {
                        QPointF( 4, 5 ),
                                            QPointF( 9, 10 ),
                                            QPointF( 14, 5 ) });

                    painter->drawPolyline(  QVector<QPointF> {
                        QPointF( 4, 9 ),
                                            QPointF( 9, 14 ),
                                            QPointF( 14, 9 ) });
                    break;

                }

                case DecorationButtonType::KeepAbove:
                {
                    painter->drawPolyline(  QVector<QPointF> {
                        QPointF( 4, 9 ),
                                            QPointF( 9, 4 ),
                                            QPointF( 14, 9 ) });

                    painter->drawPolyline(  QVector<QPointF> {
                        QPointF( 4, 13 ),
                                            QPointF( 9, 8 ),
                                            QPointF( 14, 13 ) });
                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    painter->drawRect( QRectF( 3.5, 4.5, 11, 1 ) ); // painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                    painter->drawRect( QRectF( 3.5, 8.5, 11, 1 ) ); // painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                    painter->drawRect( QRectF( 3.5, 12.5, 11, 1 ) ); // painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                    QPainterPath path;
                    path.moveTo( 5, 6 );
                    path.arcTo( QRectF( 5, 3.5, 8, 5 ), 180, -180 );
                    path.cubicTo( QPointF(12.5, 9.5), QPointF( 9, 7.5 ), QPointF( 9, 11.5 ) );
                    painter->drawPath( path );

                    painter->drawRect( QRectF( 9, 15, 0.5, 0.5 ) ); // painter->drawPoint( 9, 15 );

                    break;
                }

                default: break;
            }

        }

    }

    //__________________________________________________________________
    void Button::drawIconGnome( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        auto d = qobject_cast<Decoration*>( decoration() );

        // render background
        QColor backgroundColor;
        if ( isChecked() || this->hovered() || isHovered() )
            backgroundColor = d->titleBarColor();
        else
            backgroundColor = QColor();

        if( backgroundColor.isValid() )
        {
            if ( qGray(backgroundColor.rgb()) > 69 ) {
                painter->setPen(backgroundColor.darker(150));

                QLinearGradient gradient( 0, 0, 0, width );
                int b = 10;
                if ( isChecked() && isHovered() ) {
                    backgroundColor = backgroundColor.darker(115);
                    gradient.setColorAt(0.0, backgroundColor.lighter( 100 + 2*b ));
                    gradient.setColorAt(1.0, backgroundColor);
                }
                else if ( isChecked() ) {
                    backgroundColor = backgroundColor.darker(115);
                    gradient.setColorAt(0.0, backgroundColor.lighter( 100 + b ));
                    gradient.setColorAt(1.0, backgroundColor);
                }
                else if ( this->hovered() ) {
                    backgroundColor = backgroundColor.darker(115);
                    gradient.setColorAt(0.0, backgroundColor.lighter( 100 + 3*b ));
                    gradient.setColorAt(1.0, backgroundColor);
                }
                painter->setBrush(gradient);
                painter->drawRoundedRect( QRectF( -1, -1, 19, 19 ), 1, 1);
            }
            else{
                painter->setPen(backgroundColor.lighter(180));

                QLinearGradient gradient( 0, 0, 0, width );
                int b = 40;
                if ( isChecked() && isHovered() ) {
                    backgroundColor = backgroundColor.lighter(130);
                    gradient.setColorAt(0.0, backgroundColor.lighter( 100 + b ));
                    gradient.setColorAt(1.0, backgroundColor.darker ( 120));
                }
                else if ( isChecked() ) {
                    backgroundColor = backgroundColor.lighter(110);
                    gradient.setColorAt(0.0, backgroundColor.lighter( 100 + b ));
                    gradient.setColorAt(1.0, backgroundColor.darker ( 120 ));
                }
                else if ( this->hovered() ) {
                    backgroundColor = backgroundColor.lighter(150);
                    gradient.setColorAt(0.0, backgroundColor.lighter( 100 + b ));
                    gradient.setColorAt(1.0, backgroundColor.darker ( 120 ));
                }
                painter->setBrush(gradient);
                painter->drawRoundedRect( QRectF( -1, -1, 19, 19 ), 1, 1);
            }
        }

        // render mark
        QColor foregroundColor = d->fontColor();
        if( foregroundColor.isValid() )
        {
            // setup painter
            QPen pen( foregroundColor );
            pen.setJoinStyle( Qt::MiterJoin );
            pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

            switch( type() )
            {

                case DecorationButtonType::Close:
                {
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 6.5, 6.5 ), QPointF( 11.5, 11.5 ) );
                    painter->drawLine( QPointF( 11.5, 6.5 ), QPointF( 6.5, 11.5 ) );
                    break;
                }

                case DecorationButtonType::Maximize:
                {
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    if( isChecked() )
                        painter->drawRect(QRectF(7.5, 7.5, 3, 3));
                    else
                        painter->drawRect(QRectF(6.5, 6.5, 5, 5));
                    break;
                }

                case DecorationButtonType::Minimize:
                {
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 6.5, 11.5 ), QPointF( 11.5, 11.5 ) );
                    break;
                }

                case DecorationButtonType::OnAllDesktops:
                {

                    painter->setPen( Qt::NoPen );
                    painter->setBrush( foregroundColor );

                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    if( isChecked()) {
                        painter->drawEllipse( c, 1.0, 1.0 );
                    }
                    else {
                        painter->drawEllipse( c, 2.0, 2.0 );
                    }
                    break;
                }

                case DecorationButtonType::Shade:
                {

                    painter->setPen( pen );

                    if (isChecked())
                    {

                        painter->drawLine( 6, 12.5, 12, 12.5 );

                        QPainterPath path;
                        path.moveTo(9, 12.5);
                        path.lineTo(5, 6.5);
                        path.lineTo(13, 6.5);
                        painter->fillPath(path, QBrush(foregroundColor));


                    } else {

                        painter->drawLine( 6, 7.5, 12, 7.5 );

                        QPainterPath path;
                        path.moveTo(9, 7.5);
                        path.lineTo(5, 12.5);
                        path.lineTo(13, 12.5);
                        painter->fillPath(path, QBrush(foregroundColor));

                    }

                    break;

                }

                case DecorationButtonType::KeepBelow:
                {

                    painter->setPen( Qt::NoPen );

                    QPainterPath path;
                    path.moveTo(9, 11.5);
                    path.lineTo(5, 6.5);
                    path.lineTo(13, 6.5);
                    painter->fillPath(path, QBrush(foregroundColor));

                    break;

                }

                case DecorationButtonType::KeepAbove:
                {

                    painter->setPen( Qt::NoPen );

                    QPainterPath path;
                    path.moveTo(9, 6.5);
                    path.lineTo(5, 11.5);
                    path.lineTo(13, 11.5);
                    painter->fillPath(path, QBrush(foregroundColor));

                    break;
                }


                case DecorationButtonType::ApplicationMenu:
                {
                    painter->setPen( pen );
                    painter->setBrush( Qt::NoBrush );

                    painter->drawLine( QPointF( 6.5, 6.5 ), QPointF( 11.5, 6.5 ) );
                    painter->drawLine( QPointF( 6.5, 9 ), QPointF( 11.5, 9 ) );
                    painter->drawLine( QPointF( 6.5, 11.5 ), QPointF( 11.5, 11.5 ) );
                    break;
                }

                case DecorationButtonType::ContextHelp:
                {
                    painter->setPen( pen );

                    int startAngle = 260 * 16;
                    int spanAngle = 280 * 16;
                    painter->drawArc( QRectF( 7, 5.5, 4, 4), startAngle, spanAngle );

                    painter->setBrush( foregroundColor );

                    QPointF c = QPointF (static_cast<qreal>(9), static_cast<qreal>(12));
                    painter->drawEllipse( c, 0.5, 0.5 );

                    break;
                }

                default: break;
            }

        }

    }

    //__________________________________________________________________
    void Button::drawIconMacSierra( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        auto d = qobject_cast<Decoration*>( decoration() );
        if ( d->internalSettings()->animationsEnabled() ) {
            painter->scale( width/20, width/20 );
            painter->translate( 1, 1 );
        }
        else {
            painter->scale( 7./9.*width/20, 7./9.*width/20 );
            painter->translate( 4, 4 );
        }

        bool inactiveWindow( d && !d->window()->isActive() );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(81, 102, 107) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols color

        QColor symbolColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor)
            symbolColor = this->fontColor();
        else {
            if ( inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                symbolColor = lightSymbolColor;
            else if ( inactiveWindow && qGray(titleBarColor.rgb()) > 128 )
                symbolColor = darkSymbolColor;
            else
                symbolColor = this->autoColor( false, true, false, darkSymbolColor, lightSymbolColor );
        }

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        if ( d->internalSettings()->animationsEnabled() )
            symbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );
        else
            symbol_pen.setWidthF( 9./7.*1.7*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color;
                if ( !inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(238, 102, 90);
                else if( !inactiveWindow )
                    button_color = QColor(255, 94, 88);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() )
                {
                    painter->setPen( symbol_pen );
                    // it's a cross
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 12 ) );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 6 ) );
                }
                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color;
                if ( !inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 196, 86);
                else if( !inactiveWindow )
                    button_color = QColor(40, 200, 64);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() )
                {
                    painter->setPen( Qt::NoPen );

                    // two triangles
                    QPainterPath path1, path2;
                    if( isChecked() )
                    {
                        path1.moveTo(8.5, 9.5);
                        path1.lineTo(2.5, 9.5);
                        path1.lineTo(8.5, 15.5);

                        path2.moveTo(9.5, 8.5);
                        path2.lineTo(15.5, 8.5);
                        path2.lineTo(9.5, 2.5);
                    }
                    else
                    {
                        path1.moveTo(5, 13);
                        path1.lineTo(11, 13);
                        path1.lineTo(5, 7);

                        path2.moveTo(13, 5);
                        path2.lineTo(7, 5);
                        path2.lineTo(13, 11);
                    }

                    painter->fillPath(path1, QBrush(symbolColor));
                    painter->fillPath(path2, QBrush(symbolColor));
                }
                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color;
                if ( !inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(223, 192, 76);
                else if( !inactiveWindow )
                    button_color = QColor(255, 188, 48);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );
                }
                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(125, 209, 200);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( Qt::NoPen );
                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 6, 6, 6, 6 ) );
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(204, 176, 213);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( isChecked() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 12 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 11);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(symbolColor));

                }
                else if ( this->hovered() ) {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 6 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 7);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(255, 137, 241);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( Qt::NoPen );

                    QPainterPath path;
                    path.moveTo(9, 12);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(135, 206, 249);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( Qt::NoPen );

                    QPainterPath path;
                    path.moveTo(9, 6);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                    menuSymbolColor = this->fontColor();
                else {
                    uint r = qRed(titleBarColor.rgb());
                    uint g = qGreen(titleBarColor.rgb());
                    uint b = qBlue(titleBarColor.rgb());
                    // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
                    // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
                    // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
                    qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                    if ( colorConditional > 186 || g > 186 )
                        menuSymbolColor = darkSymbolColor;
                    else
                        menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(102, 156, 246);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( symbol_pen );
                    QPainterPath path;
                    path.moveTo( 6, 6 );
                    path.arcTo( QRectF( 5.5, 4, 7.5, 4.5 ), 180, -180 );
                    path.cubicTo( QPointF(11, 9), QPointF( 9, 6 ), QPointF( 9, 10 ) );
                    painter->drawPath( path );
                    painter->drawPoint( 9, 13 );
                }
                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconMacDarkAurorae( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        auto d = qobject_cast<Decoration*>( decoration() );
        if ( d->internalSettings()->animationsEnabled() ) {
            painter->scale( width/20, width/20 );
            painter->translate( 1, 1 );
        }
        else {
            painter->scale( 7./9.*width/20, 7./9.*width/20 );
            painter->translate( 4, 4 );
        }

        bool inactiveWindow( d && !d->window()->isActive() );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(81, 102, 107) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols color

        QColor symbolColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor)
            symbolColor = this->fontColor();
        else {
            if ( inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                symbolColor = lightSymbolColor;
            else if ( inactiveWindow && qGray(titleBarColor.rgb()) > 128 )
                symbolColor = darkSymbolColor;
            else
                symbolColor = this->autoColor( false, true, false, darkSymbolColor, lightSymbolColor );
        }

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        if ( d->internalSettings()->animationsEnabled() )
            symbol_pen.setWidthF( 1.2*qMax((qreal)1.0, 20/width ) );
        else
            symbol_pen.setWidthF( 9./7.*1.2*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color;
                if ( !inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(238, 102, 90);
                else if( !inactiveWindow )
                    button_color = QColor(255, 94, 88);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() )
                {
                    painter->setPen( symbol_pen );
                    // it's a cross
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 5 ) );
                }
                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color;
                if ( !inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 196, 86);
                else if( !inactiveWindow )
                    button_color = QColor(40, 200, 64);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() )
                {

                    painter->setPen( symbol_pen );

                    // solid vs. open rectangle
                    if( isChecked() )
                    {
                        painter->drawLine( QPointF( 4.5, 6 ), QPointF( 13.5, 6 ) );
                        painter->drawLine( QPointF( 13.5, 6 ), QPointF( 13.5, 12 ) );
                        painter->drawLine( QPointF( 4.5, 6 ), QPointF( 4.5, 12 ) );
                        painter->drawLine( QPointF( 4.5, 12 ), QPointF( 13.5, 12 ) );
                    }
                    else
                    {
                        painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
                        painter->drawLine( QPointF( 13.5, 4.5 ), QPointF( 13.5, 9 ) );
                        painter->drawLine( QPointF( 4.5, 9 ), QPointF( 4.5, 13.5 ) );
                        painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
                    }

                }
                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color;
                if ( !inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(223, 192, 76);
                else if( !inactiveWindow )
                    button_color = QColor(255, 188, 48);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );
                }
                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(125, 209, 200);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() && !isChecked() )
                {
                    painter->setPen( symbol_pen );

                    if ( !isHovered() && d->internalSettings()->animationsEnabled() ) {
                        painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 5 ) );
                        painter->drawLine( QPointF( 13, 5 ), QPointF( 13, 13 ) );
                        painter->drawLine( QPointF( 5, 5 ), QPointF( 5, 13 ) );
                        painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 13 ) );

                        painter->setBrush(QBrush(symbolColor));
                        painter->drawEllipse( c, 0.5, 0.5 );
                    }
                    else {
                        painter->drawLine( QPointF( 7, 5 ), QPointF( 15, 5 ) );
                        painter->drawLine( QPointF( 15, 5 ), QPointF( 15, 13 ) );
                        painter->drawLine( QPointF( 7, 5 ), QPointF( 7, 13 ) );
                        painter->drawLine( QPointF( 7, 13 ), QPointF( 15, 13 ) );

                        painter->drawLine( QPointF( 3, 5 ), QPointF( 3, 13 ) );
                        painter->drawLine( QPointF( 3, 5 ), QPointF( 4.5, 5 ) );
                        painter->drawLine( QPointF( 3, 13 ), QPointF( 4.5, 13 ) );
                    }

                }
                else if ( isChecked() )
                {
                    painter->setPen( symbol_pen );

                    painter->drawLine( QPointF( 5, 5 ), QPointF( 11, 5 ) );
                    painter->drawLine( QPointF( 11, 5 ), QPointF( 11, 11 ) );
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 5, 11 ) );
                    painter->drawLine( QPointF( 5, 11 ), QPointF( 11, 11 ) );

                    painter->drawLine( QPointF( 7, 7 ), QPointF( 13, 7 ) );
                    painter->drawLine( QPointF( 13, 7 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 7, 7 ), QPointF( 7, 13 ) );
                    painter->drawLine( QPointF( 7, 13 ), QPointF( 13, 13 ) );
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(204, 176, 213);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( isChecked() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 4, 12 ), QPointF( 14, 12 ) );

                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 8, 6, 2, 2 ) );
                }
                else if ( this->hovered() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 4, 6 ), QPointF( 14, 6 ) );

                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 8, 10, 2, 2 ) );
                }
                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(255, 137, 241);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawPolyline( QVector<QPointF>{
                        QPointF( 4, 7 ),
                                           QPointF( 9, 12 ),
                                           QPointF( 14, 7 ) });
                }
                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(135, 206, 249);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( symbol_pen );
                    painter->drawPolyline( QVector<QPointF>{
                        QPointF( 4, 11 ),
                                           QPointF( 9, 6 ),
                                           QPointF( 14, 11 )});
                }
                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                    menuSymbolColor = this->fontColor();
                else {
                    uint r = qRed(titleBarColor.rgb());
                    uint g = qGreen(titleBarColor.rgb());
                    uint b = qBlue(titleBarColor.rgb());
                    // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
                    // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
                    // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
                    qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                    if ( colorConditional > 186 || g > 186 )
                        menuSymbolColor = darkSymbolColor;
                    else
                        menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color;
                if ( !inactiveWindow )
                    button_color = QColor(102, 156, 246);
                else if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 100, 100);
                else
                    button_color = QColor(200, 200, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                painter->setBrush( button_color );
                painter->setPen( button_pen );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() )
                {
                    painter->setPen( symbol_pen );
                    int startAngle = 260 * 16;
                    int spanAngle = 280 * 16;
                    painter->drawArc( QRectF( 6, 4, 6, 6), startAngle, spanAngle );

                    painter->setBrush(QBrush(symbolColor));
                    r = static_cast<qreal>(1);
                    c = QPointF (static_cast<qreal>(9), static_cast<qreal>(13));
                    painter->drawEllipse( c, r, r );
                }
                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconSBEsierra( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        auto d = qobject_cast<Decoration*>( decoration() );
        if ( d->internalSettings()->animationsEnabled() ) {
            painter->scale( width/20, width/20 );
            painter->translate( 1, 1 );
        }
        else {
            painter->scale( 7./9.*width/20, 7./9.*width/20 );
            painter->translate( 4, 4 );
        }

        bool inactiveWindow( d && !d->window()->isActive() );
        bool useActiveButtonStyle( d && d->internalSettings()->buttonStyle() == 5 );
        bool useInactiveButtonStyle( d && d->internalSettings()->buttonStyle() == 6 );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(81, 102, 107) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols color

        QColor symbolColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor)
            symbolColor = this->fontColor();
        else
            symbolColor = this->autoColor( inactiveWindow, useActiveButtonStyle, useInactiveButtonStyle, darkSymbolColor, lightSymbolColor );

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        if ( d->internalSettings()->animationsEnabled() )
            symbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );
        else
            symbol_pen.setWidthF( 9./7.*1.7*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color;
                if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(238, 102, 90);
                else
                    button_color = QColor(255, 94, 88);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && this->hovered() )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }
                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    // it's a cross
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 12 ) );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 6 ) );
                }
                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color;
                if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 196, 86);
                else
                    button_color = QColor(40, 200, 64);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && this->hovered() )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }
                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( Qt::NoPen );

                    // two triangles
                    QPainterPath path1, path2;
                    if( isChecked() )
                    {
                        path1.moveTo(8.5, 9.5);
                        path1.lineTo(2.5, 9.5);
                        path1.lineTo(8.5, 15.5);

                        path2.moveTo(9.5, 8.5);
                        path2.lineTo(15.5, 8.5);
                        path2.lineTo(9.5, 2.5);
                    }
                    else
                    {
                        path1.moveTo(5, 13);
                        path1.lineTo(11, 13);
                        path1.lineTo(5, 7);

                        path2.moveTo(13, 5);
                        path2.lineTo(7, 5);
                        path2.lineTo(13, 11);
                    }

                    painter->fillPath(path1, QBrush(symbolColor));
                    painter->fillPath(path2, QBrush(symbolColor));
                }
                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color;
                if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(223, 192, 76);
                else
                    button_color = QColor(255, 188, 48);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && this->hovered() )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }
                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );
                }
                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color = QColor(125, 209, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() ||  ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( Qt::NoPen );
                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 6, 6, 6, 6 ) );
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color = QColor(204, 176, 213);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if (isChecked())
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 12 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 11);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(symbolColor));

                }
                else if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 6 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 7);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color = QColor(255, 137, 241);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() ||  ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( Qt::NoPen );

                    QPainterPath path;
                    path.moveTo(9, 12);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color = QColor(135, 206, 249);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() ||  ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( Qt::NoPen );

                    QPainterPath path;
                    path.moveTo(9, 6);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(symbolColor));
                }
                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                    menuSymbolColor = this->fontColor();
                else {
                    uint r = qRed(titleBarColor.rgb());
                    uint g = qGreen(titleBarColor.rgb());
                    uint b = qBlue(titleBarColor.rgb());
                    // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
                    // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
                    // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
                    qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                    if ( colorConditional > 186 || g > 186 )
                        menuSymbolColor = darkSymbolColor;
                    else
                        menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color = QColor(102, 156, 246);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    QPainterPath path;
                    path.moveTo( 6, 6 );
                    path.arcTo( QRectF( 5.5, 4, 7.5, 4.5 ), 180, -180 );
                    path.cubicTo( QPointF(11, 9), QPointF( 9, 6 ), QPointF( 9, 10 ) );
                    painter->drawPath( path );
                    painter->drawPoint( 9, 13 );
                }
                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconSBEdarkAurorae( QPainter *painter ) const
    {

        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        auto d = qobject_cast<Decoration*>( decoration() );
        if ( d->internalSettings()->animationsEnabled() ) {
            painter->scale( width/20, width/20 );
            painter->translate( 1, 1 );
        }
        else {
            painter->scale( 7./9.*width/20, 7./9.*width/20 );
            painter->translate( 4, 4 );
        }

        bool inactiveWindow( d && !d->window()->isActive() );
        bool useActiveButtonStyle( d && d->internalSettings()->buttonStyle() == 8 );
        bool useInactiveButtonStyle( d && d->internalSettings()->buttonStyle() == 9 );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(81, 102, 107) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols color

        QColor symbolColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor)
            symbolColor = this->fontColor();
        else
            symbolColor = this->autoColor( inactiveWindow, useActiveButtonStyle, useInactiveButtonStyle, darkSymbolColor, lightSymbolColor );

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        if ( d->internalSettings()->animationsEnabled() )
            symbol_pen.setWidthF( 1.2*qMax((qreal)1.0, 20/width ) );
        else
            symbol_pen.setWidthF( 9./7.*1.2*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color;
                if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(238, 102, 90);
                else
                    button_color = QColor(255, 94, 88);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && this->hovered() )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }
                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    // it's a cross
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 5 ) );
                }
                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color;
                if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(100, 196, 86);
                else
                    button_color = QColor(40, 200, 64);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && this->hovered() )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }
                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {

                    painter->setPen( symbol_pen );

                    // solid vs. open rectangle
                    if( isChecked() )
                    {
                        painter->drawLine( QPointF( 4.5, 6 ), QPointF( 13.5, 6 ) );
                        painter->drawLine( QPointF( 13.5, 6 ), QPointF( 13.5, 12 ) );
                        painter->drawLine( QPointF( 4.5, 6 ), QPointF( 4.5, 12 ) );
                        painter->drawLine( QPointF( 4.5, 12 ), QPointF( 13.5, 12 ) );
                    }
                    else
                    {
                        painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
                        painter->drawLine( QPointF( 13.5, 4.5 ), QPointF( 13.5, 9 ) );
                        painter->drawLine( QPointF( 4.5, 9 ), QPointF( 4.5, 13.5 ) );
                        painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
                    }

                }
                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color;
                if ( qGray(titleBarColor.rgb()) < 128 )
                    button_color = QColor(223, 192, 76);
                else
                    button_color = QColor(255, 188, 48);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && this->hovered() )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );
                }
                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color = QColor(125, 209, 200);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( !isChecked() && ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) )
                {
                    painter->setPen( symbol_pen );

                    if ( !isHovered() && d->internalSettings()->animationsEnabled() ) {
                        painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 5 ) );
                        painter->drawLine( QPointF( 13, 5 ), QPointF( 13, 13 ) );
                        painter->drawLine( QPointF( 5, 5 ), QPointF( 5, 13 ) );
                        painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 13 ) );

                        painter->setBrush(QBrush(symbolColor));
                        painter->drawEllipse( c, 0.5, 0.5 );
                    }
                    else {
                        painter->drawLine( QPointF( 7, 5 ), QPointF( 15, 5 ) );
                        painter->drawLine( QPointF( 15, 5 ), QPointF( 15, 13 ) );
                        painter->drawLine( QPointF( 7, 5 ), QPointF( 7, 13 ) );
                        painter->drawLine( QPointF( 7, 13 ), QPointF( 15, 13 ) );

                        painter->drawLine( QPointF( 3, 5 ), QPointF( 3, 13 ) );
                        painter->drawLine( QPointF( 3, 5 ), QPointF( 4.5, 5 ) );
                        painter->drawLine( QPointF( 3, 13 ), QPointF( 4.5, 13 ) );
                    }
                }
                else if ( isChecked() && ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) )
                {
                    painter->setPen( symbol_pen );

                    painter->drawLine( QPointF( 5, 5 ), QPointF( 11, 5 ) );
                    painter->drawLine( QPointF( 11, 5 ), QPointF( 11, 11 ) );
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 5, 11 ) );
                    painter->drawLine( QPointF( 5, 11 ), QPointF( 11, 11 ) );

                    painter->drawLine( QPointF( 7, 7 ), QPointF( 13, 7 ) );
                    painter->drawLine( QPointF( 13, 7 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 7, 7 ), QPointF( 7, 13 ) );
                    painter->drawLine( QPointF( 7, 13 ), QPointF( 13, 13 ) );
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color = QColor(204, 176, 213);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if (isChecked())
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 4, 12 ), QPointF( 14, 12 ) );

                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 8, 6, 2, 2 ) );
                }
                else if ( this->hovered() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 4, 6 ), QPointF( 14, 6 ) );

                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 8, 10, 2, 2 ) );
                }
                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color = QColor(255, 137, 241);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() ||  ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    painter->drawPolyline( QVector<QPointF>{
                        QPointF( 4, 7 ),
                                           QPointF( 9, 12 ),
                                           QPointF( 14, 7 ) });
                }
                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color = QColor(135, 206, 249);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() ||  ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    painter->drawPolyline( QVector<QPointF>{
                        QPointF( 4, 11 ),
                                           QPointF( 9, 6 ),
                                           QPointF( 14, 11 )});
                }
                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                    menuSymbolColor = this->fontColor();
                else {
                    uint r = qRed(titleBarColor.rgb());
                    uint g = qGreen(titleBarColor.rgb());
                    uint b = qBlue(titleBarColor.rgb());
                    // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
                    // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
                    // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
                    qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                    if ( colorConditional > 186 || g > 186 )
                        menuSymbolColor = darkSymbolColor;
                    else
                        menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color = QColor(102, 156, 246);
                QPen button_pen( qGray(titleBarColor.rgb()) < 69 ? button_color.lighter(115) : button_color.darker(115) );
                button_pen.setJoinStyle( Qt::MiterJoin );
                if ( d->internalSettings()->animationsEnabled() )
                    button_pen.setWidthF( PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );
                else
                    button_pen.setWidthF( 9./7.*PenWidth::Symbol*qMax((qreal)1.0, 20/width ) );

                if ( ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle ) && ( this->hovered() || isChecked() ) )
                {
                    // ring
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( button_pen );
                }
                else if ( ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    // nothing
                    painter->setBrush( Qt::NoBrush );
                    painter->setPen( Qt::NoPen );
                }
                else
                {
                    // filled
                    painter->setBrush( button_color );
                    painter->setPen( button_pen );
                }

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                if ( this->hovered() || isChecked() || ( inactiveWindow && !useActiveButtonStyle ) || useInactiveButtonStyle )
                {
                    painter->setPen( symbol_pen );
                    int startAngle = 260 * 16;
                    int spanAngle = 280 * 16;
                    painter->drawArc( QRectF( 6, 4, 6, 6), startAngle, spanAngle );

                    painter->setBrush(QBrush(symbolColor));
                    r = static_cast<qreal>(1);
                    c = QPointF (static_cast<qreal>(9), static_cast<qreal>(13));
                    painter->drawEllipse( c, r, r );
                }
                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconSierraColorSymbols( QPainter *painter ) const
    {
        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        auto d = qobject_cast<Decoration*>( decoration() );

        bool inactiveWindow( d && !d->window()->isActive() );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(81, 102, 107) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols color

        QColor symbolColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor)
            symbolColor = this->fontColor();
        else {
            if ( inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                symbolColor = lightSymbolColor;
            else if ( inactiveWindow && qGray(titleBarColor.rgb()) > 128 )
                symbolColor = darkSymbolColor;
            else
                symbolColor = this->autoColor( false, true, false, darkSymbolColor, lightSymbolColor );
        }

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        symbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor  button_color = QColor(238, 102, 90);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a cross
                painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 12 ) );
                painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 6 ) );

                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color = QColor(100, 196, 86);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                painter->setPen( Qt::NoPen );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);

                // two triangles
                QPainterPath path1, path2;
                if( isChecked() )
                {
                    path1.moveTo(8.5, 9.5);
                    path1.lineTo(2.5, 9.5);
                    path1.lineTo(8.5, 15.5);

                    path2.moveTo(9.5, 8.5);
                    path2.lineTo(15.5, 8.5);
                    path2.lineTo(9.5, 2.5);
                }
                else
                {
                    path1.moveTo(5, 13);
                    path1.lineTo(11, 13);
                    path1.lineTo(5, 7);

                    path2.moveTo(13, 5);
                    path2.lineTo(7, 5);
                    path2.lineTo(13, 11);
                }

                painter->fillPath(path1, QBrush(mycolor));
                painter->fillPath(path2, QBrush(mycolor));

                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color = QColor(223, 192, 76);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a horizontal line
                painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );

                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color = QColor(125, 209, 200);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                // painter->setPen( symbol_pen );
                painter->setPen( Qt::NoPen );
                painter->setBrush(QBrush(mycolor));
                painter->drawEllipse( QRectF( 6, 6, 6, 6 ) );

                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color = QColor(204, 176, 213);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker (100), symbolColor, m_opacity);
                painter->setPen( mycolor );

                // it's a triangle with a dash
                if (isChecked())
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 12 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 11);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(mycolor));

                }
                else {
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 6 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 7);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(mycolor));
                }

                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color = QColor(255, 137, 241);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker (100), symbolColor, m_opacity);
                painter->setPen( Qt::NoPen );

                // it's a downward pointing triangle
                QPainterPath path;
                path.moveTo(9, 12);
                path.lineTo(5, 6);
                path.lineTo(13, 6);
                painter->fillPath(path, QBrush(mycolor));

                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color = QColor(135, 206, 249);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                painter->setPen( Qt::NoPen );

                // it's a upward pointing triangle
                QPainterPath path;
                path.moveTo(9, 6);
                path.lineTo(5, 12);
                path.lineTo(13, 12);
                painter->fillPath(path, QBrush(mycolor));

                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                    menuSymbolColor = this->fontColor();
                else {
                    uint r = qRed(titleBarColor.rgb());
                    uint g = qGreen(titleBarColor.rgb());
                    uint b = qBlue(titleBarColor.rgb());
                    // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
                    // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
                    // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
                    qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                    if ( colorConditional > 186 || g > 186 )
                        menuSymbolColor = darkSymbolColor;
                    else
                        menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color = QColor(102, 156, 246);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a question mark
                QPainterPath path;
                path.moveTo( 6, 6 );
                path.arcTo( QRectF( 5.5, 4, 7.5, 4.5 ), 180, -180 );
                path.cubicTo( QPointF(11, 9), QPointF( 9, 6 ), QPointF( 9, 10 ) );
                painter->drawPath( path );
                painter->drawPoint( 9, 13 );

                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconDarkAuroraeColorSymbols( QPainter *painter ) const
    {
        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        auto d = qobject_cast<Decoration*>( decoration() );

        bool inactiveWindow( d && !d->window()->isActive() );
        bool isMatchTitleBarColor( d && d->internalSettings()->matchColorForTitleBar() );

        QColor darkSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(81, 102, 107) : QColor(34, 45, 50) );
        QColor lightSymbolColor( ( inactiveWindow && isMatchTitleBarColor ) ? QColor(192, 193, 194) : QColor(250, 251, 252) );

        QColor titleBarColor (d->titleBarColor());

        // symbols color

        QColor symbolColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor)
            symbolColor = this->fontColor();
        else {
            if ( inactiveWindow && qGray(titleBarColor.rgb()) < 128 )
                symbolColor = lightSymbolColor;
            else if ( inactiveWindow && qGray(titleBarColor.rgb()) > 128 )
                symbolColor = darkSymbolColor;
            else
                symbolColor = this->autoColor( false, true, false, darkSymbolColor, lightSymbolColor );
        }

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        symbol_pen.setWidthF( 1.2*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color = QColor(238, 102, 90);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a cross
                painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 5 ) );

                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color = QColor(100, 196, 86);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );

                // solid vs. open rectangle
                if( isChecked() )
                {
                    painter->drawLine( QPointF( 4.5, 6 ), QPointF( 13.5, 6 ) );
                    painter->drawLine( QPointF( 13.5, 6 ), QPointF( 13.5, 12 ) );
                    painter->drawLine( QPointF( 4.5, 6 ), QPointF( 4.5, 12 ) );
                    painter->drawLine( QPointF( 4.5, 12 ), QPointF( 13.5, 12 ) );
                }
                else
                {
                    painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
                    painter->drawLine( QPointF( 13.5, 4.5 ), QPointF( 13.5, 9 ) );
                    painter->drawLine( QPointF( 4.5, 9 ), QPointF( 4.5, 13.5 ) );
                    painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
                }

                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color = QColor(223, 192, 76);

                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a horizontal line
                painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );

                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color = QColor(125, 209, 200);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );

                if (isChecked())
                {
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 11, 5 ) );
                    painter->drawLine( QPointF( 11, 5 ), QPointF( 11, 11 ) );
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 5, 11 ) );
                    painter->drawLine( QPointF( 5, 11 ), QPointF( 11, 11 ) );

                    painter->drawLine( QPointF( 7, 7 ), QPointF( 13, 7 ) );
                    painter->drawLine( QPointF( 13, 7 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 7, 7 ), QPointF( 7, 13 ) );
                    painter->drawLine( QPointF( 7, 13 ), QPointF( 13, 13 ) );
                }
                else {
                    painter->drawLine( QPointF( 7, 5 ), QPointF( 15, 5 ) );
                    painter->drawLine( QPointF( 15, 5 ), QPointF( 15, 13 ) );
                    painter->drawLine( QPointF( 7, 5 ), QPointF( 7, 13 ) );
                    painter->drawLine( QPointF( 7, 13 ), QPointF( 15, 13 ) );

                    painter->drawLine( QPointF( 3, 5 ), QPointF( 3, 13 ) );
                    painter->drawLine( QPointF( 3, 5 ), QPointF( 4.5, 5 ) );
                    painter->drawLine( QPointF( 3, 13 ), QPointF( 4.5, 13 ) );
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color = QColor(204, 176, 213);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker (100), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                if (isChecked())
                {
                    painter->drawLine( QPointF( 4, 12 ), QPointF( 14, 12 ) );
                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 8, 6, 2, 2 ) );
                }
                else {
                    painter->drawLine( QPointF( 4, 6 ), QPointF( 14, 6 ) );
                    painter->setBrush(QBrush(mycolor));
                    painter->drawEllipse( QRectF( 8, 10, 2, 2 ) );
                }

                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color = QColor(255, 137, 241);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                painter->drawPolyline( QVector<QPointF>{
                    QPointF( 4, 7 ),
                                       QPointF( 9, 12 ),
                                       QPointF( 14, 7 ) });

                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color = QColor(135, 206, 249);
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( !isChecked() )
                    mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                painter->drawPolyline( QVector<QPointF>{
                    QPointF( 4, 11 ),
                                       QPointF( 9, 6 ),
                                       QPointF( 14, 11 )});

                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                QColor menuSymbolColor;
                bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
                if (isSystemForegroundColor)
                    menuSymbolColor = this->fontColor();
                else {
                    uint r = qRed(titleBarColor.rgb());
                    uint g = qGreen(titleBarColor.rgb());
                    uint b = qBlue(titleBarColor.rgb());
                    // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
                    // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
                    // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
                    qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
                    if ( colorConditional > 186 || g > 186 )
                        menuSymbolColor = darkSymbolColor;
                    else
                        menuSymbolColor = lightSymbolColor;
                }

                QPen menuSymbol_pen( menuSymbolColor );
                menuSymbol_pen.setJoinStyle( Qt::MiterJoin );
                menuSymbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

                painter->setPen( menuSymbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color = QColor(102, 156, 246);
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color.darker( 100 ), symbolColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a question mark

                painter->setPen( symbol_pen );
                int startAngle = 260 * 16;
                int spanAngle = 280 * 16;
                painter->drawArc( QRectF( 6, 4, 6, 6), startAngle, spanAngle );

                painter->setBrush(QBrush(symbolColor));
                r = static_cast<qreal>(1);
                c = QPointF (static_cast<qreal>(9), static_cast<qreal>(13));
                painter->drawEllipse( c, r, r );

                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconSierraMonochromeSymbols( QPainter *painter ) const
    {
        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *        scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *        this makes all further rendering and scaling simpler
         *        all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        QColor darkSymbolColor = QColor(34, 45, 50);
        QColor lightSymbolColor = QColor(250, 251, 252);

        auto d = qobject_cast<Decoration*>( decoration() );
        QColor titleBarColor (d->titleBarColor());

        QColor symbolColor;
        QColor symbolBgdColor;
        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor) {
            symbolColor = this->foregroundColor();
            symbolBgdColor = this->backgroundColor();
        } else {
            uint r = qRed(titleBarColor.rgb());
            uint g = qGreen(titleBarColor.rgb());
            uint b = qBlue(titleBarColor.rgb());

            // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
            // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
            // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
            qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
            if ( colorConditional > 186 || g > 186 ) {
                symbolColor = darkSymbolColor;
                symbolBgdColor = lightSymbolColor;
            }
            else {
                symbolColor = lightSymbolColor;
                symbolBgdColor = darkSymbolColor;
            }
        }

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        symbol_pen.setWidthF( 1.7*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = static_cast<qreal>(7)
                + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a cross
                painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 12 ) );
                painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 6 ) );

                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = static_cast<qreal>(7)
                + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );
                painter->setPen( Qt::NoPen );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);

                // two triangles
                QPainterPath path1, path2;
                if( isChecked() )
                {
                    path1.moveTo(8.5, 9.5);
                    path1.lineTo(2.5, 9.5);
                    path1.lineTo(8.5, 15.5);

                    path2.moveTo(9.5, 8.5);
                    path2.lineTo(15.5, 8.5);
                    path2.lineTo(9.5, 2.5);
                }
                else
                {
                    path1.moveTo(5, 13);
                    path1.lineTo(11, 13);
                    path1.lineTo(5, 7);

                    path2.moveTo(13, 5);
                    path2.lineTo(7, 5);
                    path2.lineTo(13, 11);
                }

                painter->fillPath(path1, QBrush(mycolor));
                painter->fillPath(path2, QBrush(mycolor));

                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = static_cast<qreal>(7)
                + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a horizontal line
                painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );

                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                if ( !isChecked() ) {
                    qreal r = static_cast<qreal>(7)
                    + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, r, r );
                }
                else {
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, 9, 9 );
                }
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( Qt::NoPen );
                painter->setBrush(QBrush(mycolor));
                painter->drawEllipse( QRectF( 6, 6, 6, 6 ) );

                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                if ( !isChecked() ) {
                    qreal r = static_cast<qreal>(7)
                    + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, r, r );
                }
                else {
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, 9, 9 );
                }
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a triangle with a dash
                if (isChecked())
                {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 12 ), QPointF( 12, 12 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 11);
                    path.lineTo(5, 6);
                    path.lineTo(13, 6);
                    painter->fillPath(path, QBrush(mycolor));
                }
                else {
                    painter->setPen( symbol_pen );
                    painter->drawLine( QPointF( 6, 6 ), QPointF( 12, 6 ) );
                    painter->setPen( Qt::NoPen );
                    QPainterPath path;
                    path.moveTo(9, 7);
                    path.lineTo(5, 12);
                    path.lineTo(13, 12);
                    painter->fillPath(path, QBrush(mycolor));
                }

                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                if ( !isChecked() ) {
                    qreal r = static_cast<qreal>(7)
                    + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, r, r );
                }
                else {
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, 9, 9 );
                }
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                painter->setPen( Qt::NoPen );

                // it's a downward pointing triangle
                QPainterPath path;
                path.moveTo(9, 12);
                path.lineTo(5, 6);
                path.lineTo(13, 6);
                painter->fillPath(path, QBrush(mycolor));

                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                if ( !isChecked() ) {
                    qreal r = static_cast<qreal>(7)
                    + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, r, r );
                }
                else {
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, 9, 9 );
                }
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                painter->setPen( Qt::NoPen );

                // it's a upward pointing triangle
                QPainterPath path;
                path.moveTo(9, 6);
                path.lineTo(5, 12);
                path.lineTo(13, 12);
                painter->fillPath(path, QBrush(mycolor));

                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                painter->setPen( symbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                if ( !isChecked() ) {
                    qreal r = static_cast<qreal>(7)
                    + static_cast<qreal>(2) * m_animation->currentValue().toReal();
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, r, r );
                }
                else {
                    QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                    painter->drawEllipse( c, 9, 9 );
                }
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );

                // it's a question mark

                QPainterPath path;
                path.moveTo( 6, 6 );
                path.arcTo( QRectF( 5.5, 4, 7.5, 4.5 ), 180, -180 );
                path.cubicTo( QPointF(11, 9), QPointF( 9, 6 ), QPointF( 9, 10 ) );
                painter->drawPath( path );
                painter->drawPoint( 9, 13 );

                break;
            }

            default: break;
        }
    }

    //__________________________________________________________________
    void Button::drawIconDarkAuroraeMonochromeSymbols( QPainter *painter ) const
    {
        painter->setRenderHints( QPainter::Antialiasing );

        /*
         *   scale painter so that its window matches QRect( -1, -1, 20, 20 )
         *   this makes all further rendering and scaling simpler
         *   all further rendering is preformed inside QRect( 0, 0, 18, 18 )
         */
        painter->translate( geometry().topLeft() );

        const qreal width( m_iconSize.width() );
        painter->scale( width/20, width/20 );
        painter->translate( 1, 1 );

        QColor darkSymbolColor = QColor(34, 45, 50);
        QColor lightSymbolColor = QColor(250, 251, 252);

        auto d = qobject_cast<Decoration*>( decoration() );
        QColor titleBarColor (d->titleBarColor());

        QColor symbolColor;
        QColor symbolBgdColor;

        bool isSystemForegroundColor( d && d->internalSettings()->systemForegroundColor() );
        if (isSystemForegroundColor) {
            symbolColor = this->foregroundColor();
            symbolBgdColor = this->backgroundColor();
        } else {
            uint r = qRed(titleBarColor.rgb());
            uint g = qGreen(titleBarColor.rgb());
            uint b = qBlue(titleBarColor.rgb());

            // modified from https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
            // qreal titleBarLuminance = (0.2126 * static_cast<qreal>(r) + 0.7152 * static_cast<qreal>(g) + 0.0722 * static_cast<qreal>(b)) / 255.;
            // if ( titleBarLuminance >  sqrt(1.05 * 0.05) - 0.05 )
            qreal colorConditional = 0.299 * static_cast<qreal>(r) + 0.587 * static_cast<qreal>(g) + 0.114 * static_cast<qreal>(b);
            if ( colorConditional > 186 || g > 186 ) {
                symbolColor = darkSymbolColor;
                symbolBgdColor = lightSymbolColor;
            }
            else {
                symbolColor = lightSymbolColor;
                symbolBgdColor = darkSymbolColor;
            }
        }

        // symbols pen

        QPen symbol_pen( symbolColor );
        symbol_pen.setJoinStyle( Qt::MiterJoin );
        symbol_pen.setWidthF( 1.2*qMax((qreal)1.0, 20/width ) );

        switch( type() )
        {

            case DecorationButtonType::Close:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a cross
                painter->drawLine( QPointF( 5, 5 ), QPointF( 13, 13 ) );
                painter->drawLine( QPointF( 5, 13 ), QPointF( 13, 5 ) );

                break;
            }

            case DecorationButtonType::Maximize:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );

                // solid vs. open rectangle
                if( isChecked() )
                {
                    painter->drawLine( QPointF( 4.5, 6 ), QPointF( 13.5, 6 ) );
                    painter->drawLine( QPointF( 13.5, 6 ), QPointF( 13.5, 12 ) );
                    painter->drawLine( QPointF( 4.5, 6 ), QPointF( 4.5, 12 ) );
                    painter->drawLine( QPointF( 4.5, 12 ), QPointF( 13.5, 12 ) );
                }
                else
                {
                    painter->drawLine( QPointF( 4.5, 4.5 ), QPointF( 13.5, 4.5 ) );
                    painter->drawLine( QPointF( 13.5, 4.5 ), QPointF( 13.5, 9 ) );
                    painter->drawLine( QPointF( 4.5, 9 ), QPointF( 4.5, 13.5 ) );
                    painter->drawLine( QPointF( 4.5, 13.5 ), QPointF( 13.5, 13.5 ) );
                }

                break;
            }

            case DecorationButtonType::Minimize:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                // it's a horizontal line
                painter->drawLine( QPointF( 5, 9 ), QPointF( 13, 9 ) );

                break;
            }

            case DecorationButtonType::OnAllDesktops:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );

                if (isChecked())
                {
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 11, 5 ) );
                    painter->drawLine( QPointF( 11, 5 ), QPointF( 11, 11 ) );
                    painter->drawLine( QPointF( 5, 5 ), QPointF( 5, 11 ) );
                    painter->drawLine( QPointF( 5, 11 ), QPointF( 11, 11 ) );

                    painter->drawLine( QPointF( 7, 7 ), QPointF( 13, 7 ) );
                    painter->drawLine( QPointF( 13, 7 ), QPointF( 13, 13 ) );
                    painter->drawLine( QPointF( 7, 7 ), QPointF( 7, 13 ) );
                    painter->drawLine( QPointF( 7, 13 ), QPointF( 13, 13 ) );
                }
                else {
                    painter->drawLine( QPointF( 7, 5 ), QPointF( 15, 5 ) );
                    painter->drawLine( QPointF( 15, 5 ), QPointF( 15, 13 ) );
                    painter->drawLine( QPointF( 7, 5 ), QPointF( 7, 13 ) );
                    painter->drawLine( QPointF( 7, 13 ), QPointF( 15, 13 ) );

                    painter->drawLine( QPointF( 3, 5 ), QPointF( 3, 13 ) );
                    painter->drawLine( QPointF( 3, 5 ), QPointF( 4.5, 5 ) );
                    painter->drawLine( QPointF( 3, 13 ), QPointF( 4.5, 13 ) );
                }
                break;
            }

            case DecorationButtonType::Shade:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                if (isChecked())
                {
                    painter->drawLine( QPointF( 4, 12 ), QPointF( 14, 12 ) );
                    painter->setBrush(QBrush(symbolColor));
                    painter->drawEllipse( QRectF( 8, 6, 2, 2 ) );
                }
                else {
                    painter->drawLine( QPointF( 4, 6 ), QPointF( 14, 6 ) );
                    painter->setBrush(QBrush(mycolor));
                    painter->drawEllipse( QRectF( 8, 10, 2, 2 ) );
                }

                break;

            }

            case DecorationButtonType::KeepBelow:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                painter->drawPolyline( QVector<QPointF>{
                    QPointF( 4, 7 ),
                                       QPointF( 9, 12 ),
                                       QPointF( 14, 7 ) });

                break;

            }

            case DecorationButtonType::KeepAbove:
            {
                QColor button_color = symbolColor;
                if ( !isChecked() )
                    button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = symbolColor;
                if ( isChecked() && !this->hovered() )
                    mycolor = this->mixColors(symbolBgdColor, button_color, m_opacity);
                else
                    mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );
                painter->drawPolyline( QVector<QPointF>{
                    QPointF( 4, 11 ),
                                       QPointF( 9, 6 ),
                                       QPointF( 14, 11 )});

                break;
            }

            case DecorationButtonType::ApplicationMenu:
            {
                painter->setPen( symbol_pen );

                painter->drawLine( QPointF( 3.5, 5 ), QPointF( 14.5, 5 ) );
                painter->drawLine( QPointF( 3.5, 9 ), QPointF( 14.5, 9 ) );
                painter->drawLine( QPointF( 3.5, 13 ), QPointF( 14.5, 13 ) );

                break;
            }

            case DecorationButtonType::ContextHelp:
            {
                QColor button_color = symbolColor;
                button_color.setAlpha( button_color.alpha()*m_opacity );
                painter->setPen( Qt::NoPen );
                painter->setBrush( button_color );

                qreal r = this->buttonRadius();
                QPointF c(static_cast<qreal>(9), static_cast<qreal>(9));
                painter->drawEllipse( c, r, r );
                painter->setBrush( Qt::NoBrush );

                button_color.setAlpha( 255 );
                symbolBgdColor.setAlpha( 255 );
                QColor mycolor = this->mixColors(button_color, symbolBgdColor, m_opacity);
                symbol_pen.setColor(mycolor);
                painter->setPen( symbol_pen );

                // it's a question mark

                painter->setPen( symbol_pen );
                int startAngle = 260 * 16;
                int spanAngle = 280 * 16;
                painter->drawArc( QRectF( 6, 4, 6, 6), startAngle, spanAngle );

                painter->setBrush(QBrush(symbolColor));
                r = static_cast<qreal>(1);
                c = QPointF (static_cast<qreal>(9), static_cast<qreal>(13));
                painter->drawEllipse( c, r, r );

                break;
            }

            default: break;
        }
    }


    //__________________________________________________________________
    QColor Button::foregroundColor(const QColor& inactiveCol) const
    {
        auto d = qobject_cast<Decoration*>(decoration());
        if (!d || d->internalSettings()->macOSButtons()) {
            QColor col;
            if (d && !d->window()->isActive()
                && !isHovered() && !isPressed()
                && m_animation->state() != QAbstractAnimation::Running)
            {
                int v = qGray(inactiveCol.rgb());
                if (v > 127) v -= 127;
                else v += 128;
                col = QColor(v, v, v);
            }
            else
            {
                if (d && qGray(d->titleBarColor().rgb()) > 100)
                    col = QColor(250, 250, 250);
                else
                    col = QColor(40, 40, 40);
            }
            return col;
        }
        else if (!d) {

            return QColor();

        } else if (isPressed()) {

            return d->titleBarColor();

        /*} else if (type() == DecorationButtonType::Close && d->internalSettings()->outlineCloseButton()) {

            return d->titleBarColor();*/

        } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove) && isChecked()) {

            return d->titleBarColor();

        } else if (m_animation->state() == QAbstractAnimation::Running) {

            return KColorUtils::mix(d->fontColor(), d->titleBarColor(), m_opacity);

        } else if (isHovered()) {

            return d->titleBarColor();

        } else {

            return d->fontColor();

        }

    }

    //__________________________________________________________________
    QColor Button::backgroundColor() const
    {
        auto d = qobject_cast<Decoration*>(decoration());
        if (!d) {

            return QColor();

        }

        if (d->internalSettings()->macOSButtons()) {
            if (isPressed()) {

                QColor col;
                if (type() == DecorationButtonType::Close)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(254, 73, 66);
                    else
                        col = QColor(240, 77, 80);
                }
                else if (type() == DecorationButtonType::Maximize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(0, 188, 154) : QColor(7, 201, 33);
                    else
                        col = isChecked() ? QColor(0, 188, 154) : QColor(101, 188, 34);
                }
                else if (type() == DecorationButtonType::Minimize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(233, 160, 13);
                    else
                        col = QColor(227, 185, 59);
                }
                else if (type() == DecorationButtonType::ApplicationMenu) {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(83, 121, 170);
                    else
                        col = QColor(110, 136, 180);
                }
                if (col.isValid())
                    return col;
                else return KColorUtils::mix(d->titleBarColor(), d->fontColor(), 0.3);

            } else if (m_animation->state() == QAbstractAnimation::Running) {

                QColor col;
                if (type() == DecorationButtonType::Close)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(254, 95, 87);
                    else
                        col = QColor(240, 96, 97);
                }
                else if (type() == DecorationButtonType::Maximize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(64, 188, 168) : QColor(39, 201, 63);
                    else
                        col = isChecked() ? QColor(64, 188, 168) : QColor(116, 188, 64);
                }
                else if (type() == DecorationButtonType::Minimize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(233, 172, 41);
                    else
                        col = QColor(227, 191, 78);
                }
                else if (type() == DecorationButtonType::ApplicationMenu) {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(98, 141, 200);
                    else
                        col = QColor(128, 157, 210);
                }
                if (col.isValid())
                    return col;
                else {

                    col = d->fontColor();
                    col.setAlpha(col.alpha()*m_opacity);
                    return col;

                }

            } else if (isHovered()) {

                QColor col;
                if (type() == DecorationButtonType::Close)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(254, 95, 87);
                    else
                        col = QColor(240, 96, 97);
                }
                else if (type() == DecorationButtonType::Maximize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = isChecked() ? QColor(64, 188, 168) : QColor(39, 201, 63);
                    else
                        col = isChecked() ? QColor(64, 188, 168) : QColor(116, 188, 64);
                }
                else if (type() == DecorationButtonType::Minimize)
                {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(233, 172, 41);
                    else
                        col = QColor(227, 191, 78);
                }
                else if (type() == DecorationButtonType::ApplicationMenu) {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(220, 124, 64);
                    else
                        col = QColor(240, 139, 96);
                }
                else {
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(98, 141, 200);
                    else
                        col = QColor(128, 157, 210);
                }
                if (col.isValid())
                    return col;
                else return d->fontColor();

            } else {

                return QColor();

            }
        }
        else {
            auto c = d->window();
            if (isPressed()) {

                if (type() == DecorationButtonType::Close) return c->color(ColorGroup::Warning, ColorRole::Foreground);
                else
                {
                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 190);
                    else
                        col = QColor(255, 255, 255, 210);
                    return col;
                }

            } else if ((type() == DecorationButtonType::KeepBelow || type() == DecorationButtonType::KeepAbove) && isChecked()) {

                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 165);
                    else
                        col = QColor(255, 255, 255, 180);
                    return col;

            } else if (m_animation->state() == QAbstractAnimation::Running) {

                if (type() == DecorationButtonType::Close)
                {

                    QColor color(c->color(ColorGroup::Warning, ColorRole::Foreground).lighter());
                    color.setAlpha(color.alpha()*m_opacity);
                    return color;

                } else {

                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 165);
                    else
                        col = QColor(255, 255, 255, 180);
                    col.setAlpha(col.alpha()*m_opacity);
                    return col;

                }

            } else if (isHovered()) {

                if (type() == DecorationButtonType::Close) return c->color(ColorGroup::Warning, ColorRole::Foreground).lighter();
                else
                {

                    QColor col;
                    if (qGray(d->titleBarColor().rgb()) > 100)
                        col = QColor(0, 0, 0, 165);
                    else
                        col = QColor(255, 255, 255, 180);
                    return col;

                }

            } else {

                return QColor();

            }
        }

    }

    //________________________________________________________________
    void Button::reconfigure()
    {

        // animation
        if (auto d = qobject_cast<Decoration*>(decoration()))
        {
            m_animation->setDuration(d->internalSettings()->animationsDuration());
            setPreferredSize(QSizeF(d->buttonSize(), d->buttonSize()));
        }

    }

    //__________________________________________________________________
    void Button::updateAnimationState(bool hovered)
    {

        auto d = qobject_cast<Decoration*>(decoration());
        if (!(d && d->internalSettings()->animationsEnabled())) return;

        QAbstractAnimation::Direction dir = hovered ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        if (m_animation->state() == QAbstractAnimation::Running && m_animation->direction() != dir)
            m_animation->stop();
        m_animation->setDirection(dir);
        if (m_animation->state() != QAbstractAnimation::Running) m_animation->start();

    }

} // namespace
