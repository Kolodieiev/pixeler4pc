#include "Pixeler.h"

#include "pixeler/setup/context_id_setup.hpp"
#include "pixeler/setup/cpu_setup.hpp"
#include "pixeler/setup/graphics_setup.hpp"
#include "pixeler/setup/ui_setup.hpp"
#include "pixeler/src/context/IContext.h"
#include "pixeler/src/driver/graphics/DisplayWrapper.h"

namespace pixeler
{
  void Pixeler::begin(uint32_t stack_depth_kb)
  {
    setCpuFrequencyMhz(BASE_CPU_FREQ_MHZ);

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
        ContextID next_context_id = context->getNextContextID();
        delete context;

        const auto it = _context_id_map.find(next_context_id);
        if (it == _context_id_map.end())
        {
          log_e("Невідомий ідентифікатор контексту: %u", next_context_id);
          esp_restart();
        }
        else
        {
          context = it->second();
        }
      }

      if (millis() - ts > WDT_GUARD_TIME)
      {
        delay(1);
        ts = millis();
      }
    }
  }
}  // namespace pixeler