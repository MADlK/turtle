// ======================================================
// Подключаем ТОЛЬКО разрешённые библиотеки
// ======================================================
#include <Windows.h>   // Потоки, Sleep, CreateThread
#include <iostream>    // Вывод в консоль
#include <cstdlib>     // Генератор случайных чисел
#include <ctime>       // Текущее время

using namespace std;

// ======================================================
// КОНСТАНТЫ ПРОГРАММЫ
// ======================================================

// Длина трассы (количество шагов до финиша)
const int TRACK_LENGTH = 50;

// Количество черепашек (и потоков)
const int TURTLE_COUNT = 5;

// Максимальное количество строк в логе событий
const int MAX_LOG_LINES = 20;

// ======================================================
// СТРУКТУРА, ОПИСЫВАЮЩАЯ ЧЕРЕПАШКУ
// ======================================================
struct Turtle
{
    int id;        // Уникальный номер черепашки
    int position;  // Текущая позиция на трассе
};

// ======================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ======================================================

// Массив всех черепашек
Turtle turtles[TURTLE_COUNT];

// Визуальное представление трассы
char track[TURTLE_COUNT][TRACK_LENGTH + 1];

// Лог событий гонки
char logLines[MAX_LOG_LINES][100];
int logCount = 0;

// Номер победителя (-1 — победитель ещё не определён)
int winner = -1;

// Критическая секция для синхронизации потоков
CRITICAL_SECTION cs;

// ======================================================
// ЛОКАЛЬНЫЙ ГЕНЕРАТОР СЛУЧАЙНЫХ ЧИСЕЛ
// (независимый для каждого потока)
// ======================================================
int LocalRandom(unsigned int& seed, int min, int max)
{
    // Линейный конгруэнтный генератор
    seed = seed * 1103515245 + 12345;

    // Возвращаем число в заданном диапазоне
    return min + (seed % (max - min + 1));
}

// ======================================================
// ОБНОВЛЕНИЕ ВИЗУАЛЬНОЙ ТРАССЫ
// ======================================================
void UpdateTrack()
{
    // Для каждой черепашки
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        // Заполняем трассу символами '-'
        for (int j = 0; j < TRACK_LENGTH; j++)
            track[i][j] = '_';

        // Завершающий символ строки
        track[i][TRACK_LENGTH] = '\0';

        // Определяем позицию черепашки
        int pos = turtles[i].position;

        // Защита от выхода за границы массива
        if (pos >= TRACK_LENGTH)
            pos = TRACK_LENGTH - 1;

        // Обозначаем черепашку символом 'T'
        track[i][pos] = 'T';
    }
}

// ======================================================
// ПЕРЕРИСОВКА КОНСОЛИ
// ======================================================
void DrawScreen()
{
    // Очистка экрана
    system("cls");

    cout << "ГОНКА ЧЕРЕПАШЕК\n\n";

    // Вывод трассы для каждой черепашки
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        cout << "Черепашка " << turtles[i].id << ": |"
            << track[i] << "|\n";
    }

    // Вывод лога событий
    cout << "\n--- СОБЫТИЯ ---\n";
    for (int i = 0; i < logCount; i++)
        cout << logLines[i] << endl;
}

// ======================================================
// ФУНКЦИЯ ПОТОКА (ОДНА ЧЕРЕПАШКА)
// ======================================================
DWORD WINAPI TurtleRun(LPVOID param)
{
    // Получаем указатель на структуру Turtle
    Turtle* turtle = (Turtle*)param;

    // Формируем уникальный seed для генератора
    unsigned int seed =
        (unsigned int)time(NULL) ^
        GetCurrentThreadId() ^
        (turtle->id * 123456);

    // Пока черепашка не дошла до финиша
    while (turtle->position < TRACK_LENGTH)
    {
        // Случайный шаг от 0 до 2
        int step = LocalRandom(seed, 0, 2);

        // Увеличиваем позицию
        turtle->position += step;

        // Не выходим за пределы трассы
        if (turtle->position > TRACK_LENGTH)
            turtle->position = TRACK_LENGTH;

        // Входим в критическую секцию
        EnterCriticalSection(&cs);
        // Обновляем трассу и выводим на экран
        UpdateTrack();
        DrawScreen();

        // Проверяем, дошла ли черепашка до финиша
        if (turtle->position >= TRACK_LENGTH)
        {
            // Добавляем сообщение в лог
            if (logCount < MAX_LOG_LINES)
            {
                wsprintfA(
                    logLines[logCount++],
                    "Черепашка %d дошла до финиша!",
                    turtle->id
                );
            }

            // Фиксируем победителя
            if (winner == -1)
                winner = turtle->id;
        }

        // Выходим из критической секции
        LeaveCriticalSection(&cs);

        // Пауза от 1 до 3 секунд
        Sleep(LocalRandom(seed, 1, 3) * 1000);
    }

    // Завершаем поток
    return 0;
}

// ======================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ======================================================
int main()
{
    // Включаем поддержку русского языка
    setlocale(LC_ALL, "ru");

    // Инициализируем критическую секцию
    InitializeCriticalSection(&cs);

    HANDLE threads[TURTLE_COUNT];

    // Инициализация данных черепашек
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        turtles[i].id = i + 1;
        turtles[i].position = 0;
    }

    // Начальная отрисовка трассы
    UpdateTrack();

    // Создание потоков (черепашек)
    for (int i = 0; i < TURTLE_COUNT; i++)
    {
        threads[i] = CreateThread(
            NULL,               // атрибуты безопасности
            0,                  // размер стека
            TurtleRun,          // функция потока
            &turtles[i],        // передаём структуру
            0,                  // старт сразу
            NULL                // ID потока не нужен
        );
    }

    // Ожидаем завершения всех потоков
    WaitForMultipleObjects(
        TURTLE_COUNT,
        threads,
        TRUE,
        INFINITE
    );

    // Входим в критическую секцию для финального вывода
    EnterCriticalSection(&cs);

    // Добавляем строку с победителем
    if (logCount < MAX_LOG_LINES)
    {
        wsprintfA(
            logLines[logCount++],
            "🏆 ПОБЕДИТЕЛЬ: Черепашка %d",
            winner
        );
    }

    // Финальная отрисовка
    DrawScreen();

    LeaveCriticalSection(&cs);

    // Освобождаем ресурсы потоков
    for (int i = 0; i < TURTLE_COUNT; i++)
        CloseHandle(threads[i]);

    // Удаляем критическую секцию
    DeleteCriticalSection(&cs);

    system("pause");
    return 0;
}