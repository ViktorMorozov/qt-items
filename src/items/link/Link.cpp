#include "Link.h"
#include <QDesktopServices>

namespace Qi
{

ViewLink::ViewLink(const QSharedPointer<ModelText>& model, ViewDefaultController createDefaultController, Qt::Alignment alignment, Qt::TextElideMode textElideMode)
    : ViewText(model, ViewDefaultControllerNone, alignment, textElideMode),
      m_pushableTracker(this)
{
    if (createDefaultController)
    {
        auto controller = QSharedPointer<ControllerMouseLink>::create();
        controller->onApply = [this] (const ItemID& item, const ControllerContext& context) {
            Q_ASSERT(item.isValid());
            if (action)
                action(item, context, this);
        };
        setController(controller);
    }
}

ViewLink::ViewLink(const QSharedPointer<ModelText>& model, const QSharedPointer<ModelUrl>& modelUrl, Qt::Alignment alignment, Qt::TextElideMode textElideMode)
    : ViewText(model, ViewDefaultControllerNone, alignment, textElideMode),
      m_pushableTracker(this)
{
    auto controller = QSharedPointer<ControllerMouseLink>::create();
    controller->onApply = [this] (const ItemID& item, const ControllerContext& context) {
        Q_ASSERT(item.isValid());
        if (action)
            action(item, context, this);
    };
    setController(controller);

    action = [modelUrl](const ItemID& item, const ControllerContext& /*context*/, const ViewLink* /*viewLink*/) {
        QDesktopServices::openUrl(modelUrl->value(item));
    };
}

void ViewLink::drawImpl(QPainter* painter, const GuiContext& ctx, const CacheContext& cache, bool* showTooltip) const
{
    PainterState pState;
    pState.save(painter);

    QColor linkColor = ctx.palette().color(ctx.colorGroup(), QPalette::Link);
    MousePushState state = m_pushableTracker.pushStateByItem(cache.item);
    switch (state)
    {
    case MousePushStateHot:
        linkColor = linkColor.lighter();
        break;

    case MousePushStatePushed:
        linkColor = linkColor.darker();
        break;

    default:;
    }

    painter->setPen(linkColor);

    QRect rect = cache.cacheView.rect();
    QString text = theModel()->value(cache.item);
    Qt::TextElideMode elideMode = textElideMode(cache.item);
    if (elideMode != Qt::ElideNone)
    {
        QString elidedText = painter->fontMetrics().elidedText(text, elideMode, rect.width());
        if (showTooltip)
            *showTooltip = (elidedText != text);

        text = elidedText;
    }
    else
    {
        if (showTooltip)
            *showTooltip = (painter->fontMetrics().width(text) > rect.width());
    }

    painter->drawText(rect, alignment(cache.item), text);

    pState.restore(painter);
}

ControllerMouseLink::ControllerMouseLink(ControllerMousePriority priority, bool processDblClick)
    : ControllerMousePushableCallback(priority, processDblClick)
{
}

void ControllerMouseLink::activateImpl(const ActivationInfo& activationInfo)
{
    ControllerMousePushableCallback::activateImpl(activationInfo);

    activationInfo.context.pushCursor(Qt::PointingHandCursor, this);
}

void ControllerMouseLink::deactivateImpl()
{
    activationState().context.popCursor(this);

    ControllerMousePushableCallback::deactivateImpl();
}


} // end namespace Qi
