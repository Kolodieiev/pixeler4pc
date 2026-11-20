#include "Pixeler.h"

#include "context/IContext.h"
#include "driver/graphics/DisplayWrapper.h"
#include "pixeler/config/cpu_config.hpp"
#include "pixeler/config/graphics_config.hpp"
#include "pixeler/config/ui_config.hpp"

namespace pixeler
{
  void Pixeler::begin(uint32_t stack_depth_kb)
  {
    (void)stack_depth_kb;
    _input.__init();

    //---------------------------------------

    sf::RenderWindow window{sf::VideoMode({UI_WIDTH, UI_HEIGHT}), WINDOW_TITLE};

    const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.setPosition({(static_cast<int>(desktop.size.x) - static_cast<int>(UI_WIDTH)) / 2,
                        (static_cast<int>(desktop.size.y) - static_cast<int>(UI_HEIGHT)) / 2});

    _display.__init(&window);

    IContext* context = new START_CONTEXT();

#ifdef TOUCHSCREEN_SUPPORT
    bool is_left_down = false;
    bool is_right_down = false;
#endif  // TOUCHSCREEN_SUPPORT

    unsigned long ts = millis();
    while (window.isOpen())
    {
      while (auto event = window.pollEvent())
      {
        if (event->is<sf::Event::Closed>())
        {
          log_i("Вікно було закрито. Вихід");
          window.close();
          delete context;
          return;
        }
        else if (const sf::Event::KeyPressed* key_pressed = event->getIf<sf::Event::KeyPressed>())
        {
          switch (key_pressed->scancode)
          {
            case sf::Keyboard::Scancode::Up:
              _input.__setState(BtnID::BTN_UP, true);
              break;
            case sf::Keyboard::Scancode::Down:
              _input.__setState(BtnID::BTN_DOWN, true);
              break;
            case sf::Keyboard::Scancode::Left:
              _input.__setState(BtnID::BTN_LEFT, true);
              break;
            case sf::Keyboard::Scancode::Right:
              _input.__setState(BtnID::BTN_RIGHT, true);
              break;
            case sf::Keyboard::Scancode::Enter:
              _input.__setState(BtnID::BTN_OK, true);
              break;
            case sf::Keyboard::Scancode::Backspace:
              _input.__setState(BtnID::BTN_BACK, true);
              break;
            default:
              break;
          }
        }
        else if (const sf::Event::KeyReleased* key_released = event->getIf<sf::Event::KeyReleased>())
        {
          switch (key_released->scancode)
          {
            case sf::Keyboard::Scancode::Up:
              _input.__setState(BtnID::BTN_UP, false);
              break;
            case sf::Keyboard::Scancode::Down:
              _input.__setState(BtnID::BTN_DOWN, false);
              break;
            case sf::Keyboard::Scancode::Left:
              _input.__setState(BtnID::BTN_LEFT, false);
              break;
            case sf::Keyboard::Scancode::Right:
              _input.__setState(BtnID::BTN_RIGHT, false);
              break;
            case sf::Keyboard::Scancode::Enter:
              _input.__setState(BtnID::BTN_OK, false);
              break;
            case sf::Keyboard::Scancode::Backspace:
              _input.__setState(BtnID::BTN_BACK, false);
              break;
            default:
              break;
          }
        }
#ifdef TOUCHSCREEN_SUPPORT
        else if (const sf::Event::MouseButtonPressed* mouse_pressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
          if (mouse_pressed->button == sf::Mouse::Button::Left)
            is_left_down = true;
          else if (mouse_pressed->button == sf::Mouse::Button::Right)
            is_right_down = true;

          if (is_left_down || is_right_down)
            _input.__setTouchState(true, mouse_pressed->position.x, mouse_pressed->position.y);
        }
        else if (const sf::Event::MouseButtonReleased* mouse_released = event->getIf<sf::Event::MouseButtonReleased>())
        {
          if (mouse_released->button == sf::Mouse::Button::Left)
            is_left_down = false;
          else if (mouse_released->button == sf::Mouse::Button::Right)
            is_right_down = false;

          _input.__setTouchState(is_left_down || is_right_down, mouse_released->position.x, mouse_released->position.y);
        }
        else if (const sf::Event::MouseMoved* mouse_moved = event->getIf<sf::Event::MouseMoved>())
        {
          if (is_left_down || is_right_down)
            _input.__setTouchState(true, mouse_moved->position.x, mouse_moved->position.y);
        }
        else if (event->is<sf::Event::FocusLost>())
        {
          if (is_left_down || is_right_down)
          {
            is_left_down = false;
            is_right_down = false;
            _input.__setTouchState(false, 0, 0);
          }
        }
#endif  // TOUCHSCREEN_SUPPORT
      }

      if (!context->isReleased())
      {
        context->tick();
      }
      else
      {
        IContext* next_context = context->takeNextContext();

        if (!next_context)
        {
          log_e("Наступний контекст першого рівня не може бути null");
          esp_restart();
        }

        delete context;
        context = next_context;
      }

      if (millis() - ts > WDT_GUARD_TIME)
      {
        delay(1);
        ts = millis();
      }
    }

    delete context;
  }
}  // namespace pixeler
