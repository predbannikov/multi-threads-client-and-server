# Низкоуровневое приложение клиент-сервер.

Клиент посылает комманды серверу в отдельном потоке, ждёт выполнения, после чего завершается или продолжает работать с сервером.
Сервер обрабатывает полученные команды. Все команды выполняются изолированно в своих потоках.

## first rel

linux-image-4.15.0-142-generic, gcc version 5.4.0, STL, c++14, cmake 3.5.1
