server:main.cpp xserver.h xserver.cpp ./threadpool/threadpool.h ./http/http_conn.cpp ./http/http_conn.h \
./lock/locker.h ./log/log.cpp ./log/log.h ./log/block_queue.h ./CGImysql/sql_connection_pool.cpp \
./CGImysql/sql_connection_pool.h ./CGImysql/sqlcon.h ./CGImysql/sqlcon.cpp
	g++ -o server main.cpp xserver.cpp xserver.h ./threadpool/threadpool.h \
	./http/http_conn.cpp ./http/http_conn.h ./lock/locker.h ./log/log.cpp ./log/log.h \
	./CGImysql/sql_connection_pool.cpp ./CGImysql/sql_connection_pool.h ./CGImysql/sqlcon.cpp ./CGImysql/sqlcon.h -lpthread -lmysqlclient -std=c++11 -g


clean:
	rm  -r server
