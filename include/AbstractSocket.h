#ifndef ABSTRACT_LOCAL_SOCKET_H
#define ABSTRACT_LOCAL_SOCKET_H

class AbstractSocket {
    public:
        virtual ~AbstractSocket() {}

        virtual void shutdown() = 0;
        virtual void close() = 0;
};

#endif /* ABSTRACT_LOCAL_SOCKET_H */
