#include "KeyListener.hpp"
#include "editor/Editor.hpp"
#include "util/Util.hpp"

#include <QDebug>

#define GEN_KEY64(X, Y) (static_cast<qint64>(X) | (static_cast<qint64>(Y) << 32))
constexpr bool ERROR = false;
constexpr bool SUCCESS = true;

namespace pico {

KeyListener::KeyListener(QObject *parent)
    : QObject(parent),
      m_keyMap(keymap_t{}),
      m_keyMapIndex(&m_keyMap)
{}

bool
KeyListener::handleKeyPress(key64_t key)
{
    auto editor = Editor::getInstance();

    key |= Qt::SHIFT * editor->shiftState();
    key |= Qt::CTRL * editor->controlState();
    key |= Qt::ALT * editor->altState();

    auto it = m_keyMapIndex->find(GEN_KEY64(key, editor->mode()));

    if (it != m_keyMapIndex->end()) {
        value_t &val = it->second;
        if (val.callable) {
            val.callback();
            resetMapIndex();
        } else {
            /* traverse the keymap-tree */
            m_keyMapIndex = val.next;
        }
        return true;
    } else {
        resetMapIndex();
        return false;
    }
}

bool
KeyListener::addBinding(QList<QKeyCombination> keyCombo, Mode mode, callback_t callback)
{
    if (keyCombo.isEmpty())
        return ERROR;
    Q_ASSERT(m_keyMapIndex == &m_keyMap);

    key64_t key;
    QList<QKeyCombination>::iterator it_key;
    /* setup the binding tree */
    for (it_key = keyCombo.begin(); it_key < keyCombo.end() - 1; it_key++) {
        key = GEN_KEY64(it_key->toCombined(), mode);

        auto res = m_keyMapIndex->emplace(std::move(key), new keymap_t).first;
        value_t &value = res->second;

        if (!value.callable)
            m_keyMapIndex = value.next;
        else
            goto err;
    }
    /* put the callback on leaf node */
    key = GEN_KEY64(it_key->toCombined(), mode);

    if (m_keyMapIndex->find(key) == m_keyMapIndex->end())
        m_keyMapIndex->emplace(std::move(key), callback_t{ std::move(callback) });
    else
        goto err;

    resetMapIndex();
    return SUCCESS;
err:
    qWarning() << "Binding:" << keyCombo << "failed due to confilcting key combinations";
    resetMapIndex();
    return ERROR;
}

void
KeyListener::resetMapIndex(void)
{
    m_keyMapIndex = &m_keyMap;
}

} // namespace pico
