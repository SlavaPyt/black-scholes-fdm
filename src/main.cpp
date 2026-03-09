#include "bs.hpp"

#include <fstream>
#include <iostream>
#include <string>

struct Outputs {
    double price = 0.0;
    double delta = 0.0;
    double gamma = 0.0;
    double vega  = 0.0;
};

Outputs compute(const BlackScholes& model, const std::string& style);

int main(int argc, char* argv[])
{
    std::cout << "Black-Scholes project started\n";

    // ============================================================
    // 1) Параметры модели Black–Scholes (значения по умолчанию)
    //
    // S     — текущая цена базового актива
    // K     — страйк
    // r     — безрисковая ставка
    // sigma — волатильность
    // T     — время до экспирации (в годах)
    // ============================================================
    double S     = 100.0;
    double K     = 100.0;
    double r     = 0.0;
    double sigma = 0.2;
    double T     = 1.0;

    // ============================================================
    // 2) Режимы/настройки инструмента
    //
    // style  — тип опциона ("call" / "put")
    // method — метод (пока только "bs", позже добавим "crr" и т.п.)
    // ============================================================
    std::string style  = "call";
    std::string method = "bs";

    // ============================================================
    // 3) Grid-режим: хотим посчитать цену на сетке значений S
    //
    // grid_mode  — включён ли режим сетки
    // grid_start — начало диапазона S
    // grid_end   — конец диапазона S
    // grid_n     — количество точек сетки
    // ============================================================
    bool   grid_mode  = false;
    double grid_start = 50.0;
    double grid_end   = 150.0;
    int    grid_n     = 101;

    // ============================================================
    // 4) Вывод результатов
    //
    // out_path — путь к CSV-файлу для grid-mode
    // ============================================================
    std::string out_path = "";

    // ============================================================
    // 5) Парсинг аргументов командной строки (CLI)
    //
    // (здесь производится чтение аргументов пользователя)
    // ============================================================
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--S") {
            // Защита от ситуации: пользователь написал "--S" и забыл значение
            if (i + 1 < argc) {
                S = std::stod(argv[i + 1]);
                i++;
            } else {
                std::cout << "Error: --S requires a value\n";
                return 1;
            }
        }
        else if (arg == "--K") {
            K = std::stod(argv[i + 1]);
            i++;
        }
        else if (arg == "--r") {
            r = std::stod(argv[i + 1]);
            i++;
        }
        else if (arg == "--sigma") {
            sigma = std::stod(argv[i + 1]);
            i++;
        }
        else if (arg == "--T") {
            T = std::stod(argv[i + 1]);
            i++;
        }
        else if (arg == "--grid-S") {
            // Формат: --grid-S start end n
            grid_start = std::stod(argv[i + 1]);
            grid_end   = std::stod(argv[i + 2]);
            grid_n     = std::stoi(argv[i + 3]);

            grid_mode = true;
            i += 3; // мы "съели" ещё 3 аргумента
        }
        else if (arg == "--style") {
            // Формат: --style call|put
            style = argv[i + 1];
            i++;
        }
        else if (arg == "--method") {
            // Формат: --method bs|crr (пока реализован только bs)
            method = argv[i + 1];
            i++;
        }
        else if (arg == "--out") {
            // Формат: --out output.csv
            out_path = argv[i + 1];
            i++;
        }
        // else {
        //     Можно позже добавить обработку неизвестных аргументов:
        //     std::cout << "Unknown arg: " << arg << "\n";
        // }
    }

    // ============================================================
    // 6) Выполнение: single-mode (одна цена) или grid-mode (таблица)
    // ============================================================

    // ------------------------------
    // 6A) Single-mode: одна цена
    // ------------------------------
    if (!grid_mode) {
        BSParams p{S, K, r, sigma, T};
        BlackScholes model (p);

        auto opt = make_option(style, model);
        Quote q = opt->quote();

        std::cout << "price=" << q.price << "\n";
        std::cout << "delta=" << q.delta << "\n";
        std::cout << "gamma=" << q.gamma << "\n";
        std::cout << "vega="  << q.vega  << "\n";

        return 0;
    }

    // ------------------------------
    // 6B) Grid-mode: CSV с ценами на сетке S
    // ------------------------------
    std::cout << "GRID MODE\n";
    std::cout << "S from " << grid_start << " to " << grid_end
              << " with n=" << grid_n << "\n";

    // В grid-mode файл обязателен, иначе некуда сохранять результаты
    if (out_path.empty()) {
        std::cout << "Error: --out required in grid mode\n";
        return 1;
    }

    // Открываем файл для записи
    std::ofstream file(out_path);
    if (!file.is_open()) {
        std::cout << "Error: cannot open file\n";
        return 1;
    }

    // Заголовок CSV
    file << "S,price\n";

    // Шаг сетки: n точек => n-1 интервалов
    double step = (grid_end - grid_start) / (grid_n - 1);

    // Идём по точкам сетки и сохраняем (S, price)
    for (int j = 0; j < grid_n; ++j) {
        double S_j = grid_start + j * step;

        BSParams p{S_j, K, r, sigma, T};

        BlackScholes model(p);

        auto opt = make_option(style, model);

        Quote q = opt->quote();

        file << S_j << "," << q.price;
    }
    // file закроется автоматически при выходе из main

    return 0;
}