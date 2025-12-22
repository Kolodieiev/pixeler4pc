-- Скрипт демонструє обробку стану кнопок.

-- Номера пінів кнопок, як вказано в прошивці Pixeler.
BTN_UP = 4
BTN_LEFT = 2
BTN_RIGHT = 3
BTN_EXIT = 1 

function update() -- Функція викликається автоматично контекстом кожен кадр.
	if input.is_pressed(BTN_EXIT) then 
        input.lock(BTN_EXIT, 1000) -- Заблокувати спрацьовування кнопки на n мс.
        print("EXIT");
		context.exit()
    elseif input.is_pressed(BTN_UP) then -- Обробка стану is_pressed повинна завжди виконуватися ДО is_holded на одній кнопці.
        input.lock(BTN_UP, 1000)
        print("Кнопка \"вгору\" затиснута тривалий час");
    elseif input.is_released(BTN_UP) then
        input.lock(BTN_UP, 100)
        print("Відпущено кнопку \"вгору\"");
    elseif input.is_holded(BTN_RIGHT) then
        input.lock(BTN_RIGHT, 100)
        print("Утримується кнопка \"вправо\"");
    end
	
end