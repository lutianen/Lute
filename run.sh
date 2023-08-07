# Start service
service mysql start
service redis-server start

# Start application
./build/bin/httpServer \
    LuxPolaris \
    ../../app/HTML 5836 8 \
    127.0.0.1 3306 lutianen lutianen LuxDatabase
