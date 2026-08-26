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
    _input.__init();

    //---------------------------------------

    sf::RenderWindow window{sf::VideoMode({UI_WIDTH, UI_HEIGHT}), WINDOW_TITLE};

    _display.__init(&window);

    IContext* context = new START_CONTEXT();

    unsigned long ts = millis();
    while (window.isOpen())
    {
      while (auto event = window.pollEvent())
      {
        if (event->is<sf::Event::Closed>())
        {
          log_i("Вікно було закрито. Вихід");
          window.close();
          return;
        }
        else if (event->is<sf::Event::KeyPressed>())
        {
          if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::Up)
            _input.__setState(BtnID::BTN_UP, true);
          else if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::Down)
            _input.__setState(BtnID::BTN_DOWN, true);
          else if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::Left)
            _input.__setState(BtnID::BTN_LEFT, true);
          else if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::Right)
            _input.__setState(BtnID::BTN_RIGHT, true);
          else if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::Enter)
            _input.__setState(BtnID::BTN_OK, true);
          else if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::Backspace)
            _input.__setState(BtnID::BTN_BACK, true);
        }
        else if (event->is<sf::Event::KeyReleased>())
        {
          if (event->getIf<sf::Event::KeyReleased>()->scancode == sf::Keyboard::Scancode::Up)
            _input.__setState(BtnID::BTN_UP, false);
          else if (event->getIf<sf::Event::KeyReleased>()->scancode == sf::Keyboard::Scancode::Down)
            _input.__setState(BtnID::BTN_DOWN, false);
          else if (event->getIf<sf::Event::KeyReleased>()->scancode == sf::Keyboard::Scancode::Left)
            _input.__setState(BtnID::BTN_LEFT, false);
          else if (event->getIf<sf::Event::KeyReleased>()->scancode == sf::Keyboard::Scancode::Right)
            _input.__setState(BtnID::BTN_RIGHT, false);
          else if (event->getIf<sf::Event::KeyReleased>()->scancode == sf::Keyboard::Scancode::Enter)
            _input.__setState(BtnID::BTN_OK, false);
          else if (event->getIf<sf::Event::KeyReleased>()->scancode == sf::Keyboard::Scancode::Backspace)
            _input.__setState(BtnID::BTN_BACK, false);
        }
      }

      if (!context->isReleased())
      {
        context->tick();
      }
      else
      {
        IContext* next_context = context->takeNextContext();

        if (!next_context) [[unlikely]]
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
  }
}  // namespace pixeler
