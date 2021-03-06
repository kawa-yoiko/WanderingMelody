#include "gig/MusicianNKeys.h"
#include "gig/Gig.h"

#include "cocos2d.h"
#include "Global.h"
#include "Config.h"
using namespace cocos2d;

template <int N> typename MusicianNKeys<N>::Display *
    MusicianNKeys<N>::Display::create(MusicianNKeys<N> *mus)
{
    auto ret = new MusicianNKeys<N>::Display();
    if (ret == nullptr || !ret->init(mus)) {
        delete ret;
        return nullptr;
    }
    return ret;
}

template <int N> bool MusicianNKeys<N>::Display::init(MusicianNKeys<N> *mus)
{
    _mus = mus;

    _drawNode = DrawNode::create();
    if (!_drawNode) return false;
    this->addChild(_drawNode);

    auto keyboardListener = cocos2d::EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event *event) {
        _mus->sendEvent((int)keyCode);
    };
    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event) {
        _mus->sendEvent((int)keyCode + 1048576);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    auto controllerListener = EventListenerController::create();
    controllerListener->onConnected = [](Controller *controller, Event *event) {
        printf("!connected %s\n", controller->getDeviceName().c_str());
    };
    controllerListener->onKeyDown = [this](Controller *controller, int keyCode, Event *event) {
        printf("!down %d %d\n", keyCode, gamepadCode(controller->getDeviceId(), keyCode));
        _mus->sendEvent(gamepadCode(controller->getDeviceId(), keyCode));
    };
    controllerListener->onKeyUp = [this](Controller *controller, int keyCode, Event *event) {
        printf("!up %d\n", keyCode);
        _mus->sendEvent(gamepadCode(controller->getDeviceId(), keyCode) + 1048576);
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(controllerListener, this);

    for (int i = 0; i < _mus->_barlines.size(); i++) {
        char s[8];
        snprintf(s, sizeof s, "%d", i + 1);
        auto label = Label::createWithTTF(s, "OpenSans-Light.ttf", 32);
        label->setHeight(40);
        label->setColor(Color3B(179, 179, 179));
        label->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
        label->setPosition(Vec2(6, 0));
        _drawNode->addChild(label);
        _labels.push_back(label);
    }

    return true;
}

template <int N> void MusicianNKeys<N>::Display::refresh()
{
    _drawNode->clear();

    Size size = this->getContentSize();
    float speed = Config::getScrollSpeed() / _mus->getOrigTempo();

    _drawNode->drawSolidRect(
        Vec2(0, 0),
        Vec2(size.width, size.height),
        Color4F(0.8, 0.8, 0.8, 0.8));

    for (int i = 1; i < N; i++)
        _drawNode->drawSegment(
            Vec2(size.width * i / N, 0),
            Vec2(size.width * i / N, size.height),
            1,
            Color4F(0.7, 0.7, 0.7, 0.5)
        );

    for (int i = 0; i < N; i++) if (_mus->_isKeyDown[i])
        _drawNode->drawSolidRect(
            Vec2(size.width * i / N, 0),
            Vec2(size.width * (i + 1) / N, size.height),
            Color4F(0.8, 0.8, 0.8, 1));

    // Bar lines
    for (int i = 0; i < _mus->_barlines.size(); i++) {
        float posY = HIT_LINE_POS +
            size.height * speed * (_mus->_barlines[i] - _mus->getCurTick());
        _drawNode->drawSegment(
            Vec2(0, posY), Vec2(size.width, posY), 2, Color4F(0.7, 0.7, 0.7, 0.5)
        );
        _labels[i]->setVisible(_mus->_isRehearsal);
        _labels[i]->setPositionY(posY);
    }

    // Progress indicators
    const Color4F playerColors[4] = {
        {0.8f, 0.8f, 0.5f, 0.9f},
        {1.0f, 0.7f, 1.0f, 0.9f},
        {0.5f, 1.0f, 1.0f, 0.9f},
        {0.7f, 1.0f, 0.7f, 0.9f}
    };

    const float TRI_SIZE = 12;
    const float SQRT3_2 = 0.8660254037844386;

    int myId = -1;
    for (int i = 0; i < _mus->getGig()->getMusicianCount(); i++) {
        Musician *mus = _mus->getGig()->getMusician(i);
        if (mus == _mus) {
            myId = i;
        } else {
            double p = mus->getCurTick();
            float posY = HIT_LINE_POS + size.height * speed * (p - _mus->getCurTick());
            posY = clampf(posY, 12, size.height - 12);
            _drawNode->drawTriangle(
                Vec2(-3, posY),
                Vec2(-3 - TRI_SIZE * SQRT3_2, posY + TRI_SIZE / 2),
                Vec2(-3 - TRI_SIZE * SQRT3_2, posY - TRI_SIZE / 2),
                playerColors[i]
            );
        }
    }

    _drawNode->drawSegment(
        Vec2(0, HIT_LINE_POS),
        Vec2(size.width, HIT_LINE_POS),
        2, playerColors[myId]);

    // Notes
    for (const auto &n : _mus->_keyNotes) {
        float posX = size.width / N * n.track;
        float posY = HIT_LINE_POS + size.height * speed * (n.time - _mus->getCurTick());
        _drawNode->drawSegment(
            Vec2(posX + 2, posY),
            Vec2(posX + size.width / N - 2, posY),
            2,
            n.triggered == -1 ? Color4F(0.5, 0.6, 1, 0.9) : Color4F(1, 0.8, 0.4, 0.9));
    }
}

template class MusicianNKeys<2>;
template class MusicianNKeys<4>;
