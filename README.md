Небольшой "веб-сервер". Поддерживает только команду get и протокол HTTP 1.0.
Основная особенность данного проекта в том, что в пуле потоков применен epoll.
В каждый из потоков передается слушающий сокет, затем (в каждом из потоков) делается вызов epoll_create(...), затем epoll_wait(...).
Таким образом каждый из потоков обрабатывает свой наборе клиентов.
