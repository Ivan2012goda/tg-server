#include "core/launcher.h"
#include "httplib.h"
#include <thread>
#include <QtCore/QMetaObject>
#include <QtCore/QCoreApplication>

// Функция, которая выполняется в отдельном потоке
void StartLocalHttpServer() {
    httplib::Server svr;

    // Тестовый эндпоинт для проверки связи
    svr.Get("/ping", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("pong", "text/plain");
    });

    // Эндпоинт для приема команд от Python
    svr.Post("/api/cmd", [](const httplib::Request &req, httplib::Response &res) {
        // Извлекаем тело или параметры запроса
        std::string payload = req.body;

        // ВАЖНО: передаем задачу в главный поток Qt (где крутится UI Telegram)
        QMetaObject::invokeMethod(QCoreApplication::instance(), [payload]() {
            // Внутри этого блока можно безопасно обращаться к объектам Qt / tdesktop
            // Например: вызывать методы обновления интерфейса
        }, Qt::QueuedConnection);

        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // Слушаем только локальный интерфейс (localhost) на порту 8080
    svr.listen("127.0.0.1", 8080);
}

int main(int argc, char *argv[]) {
    // 1. Запускаем HTTP-сервер асинхронно до инициализации GUI
    std::thread(StartLocalHttpServer).detach();

    // 2. Запускаем стандартный цикл Telegram Desktop
    const auto launcher = Core::Launcher::Create(argc, argv);
    return launcher ? launcher->exec() : 1;
}